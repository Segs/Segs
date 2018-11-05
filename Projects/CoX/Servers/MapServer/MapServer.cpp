/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapServer.h"

#include "Servers/HandlerLocator.h"
#include "ConfigExtension.h"
#include "MapManager.h"
#include "GameData/GameDataStore.h"
#include "Runtime/RuntimeData.h"
#include "MapTemplate.h"
#include "MapInstance.h"
#include "MapClientSession.h"
#include "SEGSTimer.h"
#include "Settings.h"
#include "Servers/MessageBus.h"
#include "MessageHelpers.h"

#include "SEGSEventFactory.h"
#include "Messages/UserRouterService/UserRouterEvents.h"

#include <ace/Reactor.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <set>

using namespace SEGSEvents;

// global variables
MapServer *g_GlobalMapServer=nullptr;

// anonymous namespace
namespace
{
    constexpr int                MAPSERVER_VERSION=1;
} // end of anonymous namespace

///////////////////////////////////////////////////////////////////////////////
/// \brief The MapServer::PrivateData class - PIMPL idiom for internal
/// MapServer data
///
class MapServer::PrivateData
{
public:
    MapManager      m_manager;
};

MapServer::MapServer(uint8_t id) : d(new PrivateData), m_id(id), m_message_bus_endpoint(*this)
{
    assert(g_GlobalMapServer==nullptr && "Only one GameServer instance per process allowed");
    g_GlobalMapServer = this;

    HandlerLocator::setMap_Handler(id,this);

    m_message_bus_endpoint.subscribe(evClientConnectedMessage);
    m_message_bus_endpoint.subscribe(evClientDisconnectedMessage);
}

MapServer::~MapServer()
{
}

bool MapServer::Run()
{
    assert(m_owner_game_server_id != INVALID_GAME_SERVER_ID);

    if(!getGameData().read_game_data(RUNTIME_DATA_PATH))
        return false;

    if(!getRuntimeData().prepare(RUNTIME_DATA_PATH))
        return false;
    assert(d->m_manager.num_templates() > 0);

    qInfo() << "MapServer" << m_id << "now listening on" << m_base_listen_point.get_host_addr() << ":"
            << m_base_listen_point.get_port_number();

    return true;
}

/**
 * @param  inipath Doc at RoamingServer::ReadConfig
 * @return bool (false means an error occurred )
 */
bool MapServer::ReadConfigAndRestart()
{
    qInfo() << "Loading MapServer settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    // tell all instances to shut down
    d->m_manager.shut_down_all();

    config.beginGroup("MapServer");
    if(!config.contains(QStringLiteral("listen_addr")))
        qDebug() << "Config file is missing 'listen_addr' entry in MapServer group, will try to use default";
    if(!config.contains(QStringLiteral("location_addr")))
        qDebug() << "Config file is missing 'location_addr' entry in MapServer group, will try to use default";
    if(!config.contains(QStringLiteral("player_fade_in")))
        qDebug() << "Config file is missing 'player_fade_in' entry in MapServer group, will try to use default";

    QString listen_addr = config.value("listen_addr","127.0.0.1:7003").toString();
    QString location_addr = config.value("location_addr","127.0.0.1:7003").toString();

    QString map_templates_dir = config.value("maps",".").toString();
    if(!parseAddress(listen_addr,m_base_listen_point))
    {
        qCritical() << "Badly formed IP address" << listen_addr;
        return false;
    }
    if(!parseAddress(location_addr,m_base_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }

    bool ok = true;
    QVariant fade_in_variant = config.value("player_fade_in","380.0");
    getGameData().m_player_fade_in = fade_in_variant.toFloat(&ok);
    if(!ok)
    {
        qCritical() << "Badly formed float for 'player_fade_in': " << fade_in_variant.toString();
        return false;
    }

    QVariant motd_timer = config.value("motd_timer","120.0");
    getGameData().m_motd_timer = motd_timer.toFloat(&ok);
    if(!ok)
    {
        qCritical() << "Badly formed float for 'motd_timer': " << motd_timer.toString();
        return false;
    }

    // get costume slot unlock levels for use in finalizeLevel()
    getGameData().m_costume_slot_unlocks = config.value(QStringLiteral("costume_slot_unlocks"), "19,29,39,49").toString().remove(QRegExp("\\s")).split(',');

    config.endGroup(); // MapServer

    if(!d->m_manager.load_templates(map_templates_dir,m_owner_game_server_id,m_id,{m_base_listen_point,m_base_location}))
    {
        postGlobalEvent(new ServiceStatusMessage({ QString("MapServer: Cannot load map templates from %1").arg(map_templates_dir),-1 },0));
        return false;
    }
    return Run();
}

void MapServer::per_thread_shutdown()
{
    qWarning() << "Shutting down map server";
    // tell all instances to shut down too
    d->m_manager.shut_down_all();
}

MapManager &MapServer::map_manager()
{
    return d->m_manager;
}

void MapServer::set_game_server_owner(uint8_t owner_id)
{
    m_owner_game_server_id = owner_id;
}

void MapServer::dispatch(Event *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case evReloadConfigMessage:
            ReadConfigAndRestart();
            break;
        case Internal_EventTypes::evExpectMapClientRequest:
            on_expect_client(static_cast<ExpectMapClientRequest *>(ev));
            break;
        case Internal_EventTypes::evClientMapXferMessage:
            on_client_map_xfer(static_cast<ClientMapXferMessage *>(ev));
            break;
        case evUserRouterInfoMessage:
            route_info_message(static_cast<UserRouterInfoMessage *>(ev));
            break;
        case evUserRouterOpaqueRequest:
            fill_opaque_message(static_cast<UserRouterOpaqueRequest *>(ev));
            route_opaque_message(static_cast<UserRouterOpaqueRequest *>(ev));
            break;
        // will be obtained from MessageBusEndpoint
        case Internal_EventTypes::evClientConnectedMessage:
			on_client_connected(static_cast<ClientConnectedMessage *>(ev));
			break;
        case Internal_EventTypes::evClientDisconnectedMessage:
			on_client_disconnected(static_cast<ClientDisconnectedMessage *>(ev));
			break;
        case UserRouterEventTypes::evUserRouterQueryRequest:
            on_user_router_query_request(static_cast<UserRouterQueryRequest *>(ev));
            break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}

MapClientSession* MapServer::getMapClientSessionForEntityID(uint32_t entity_id)  
{
	MapInstance *mi = getMapInstanceForEntityID(entity_id);

	if (mi == nullptr)
		return nullptr;

	for (MapClientSession *cl : mi->m_session_store)
	{
		if (cl->m_ent->m_db_id == entity_id) {
			return cl;
		}
	}

	return nullptr;
}

MapInstance *MapServer::getMapInstanceForEntityID(uint32_t entity_id)
{
	return m_id_to_map_instance[entity_id];
}

QString MapServer::getNameFromMapInstance(uint32_t entity_id)
{
	MapInstance *mi = m_id_to_map_instance[entity_id];
	qCritical() << "mi" << mi;
	for (MapClientSession *cl : mi->m_session_store)
	{
		qCritical() << "cl" << cl;
		if (cl->m_ent->m_db_id == entity_id) {
			return cl->m_ent->name();
		}
	}

	return 	"";
}

void MapServer::on_user_router_query_request(UserRouterQueryRequest *msg)
{
    qCritical() << " got request" << msg->m_data.m_query_id << msg->m_data.m_query_name;

    QString _name = "";
    uint32_t _id = 0;

    if (msg->m_data.m_query_id != 0)
		_id = msg->m_data.m_query_id;
	else
		_name = msg->m_data.m_query_name;

	for (EventProcessor *_mi : HandlerLocator::m_map_instances[m_id])
	{
		if (_mi == nullptr)
			continue;

		MapInstance *mi = static_cast<MapInstance *>(_mi);
		for (MapClientSession *cl : mi->m_session_store)
		{
			if (_id == 0) 
			{
				if (cl->m_ent->name() == _name) 
					_id = cl->m_ent->m_db_id;
			}
			else if (cl->m_ent->m_db_id == _id) 
				_name = cl->m_ent->name();
		}
	}

	assert(_id != 0);
	assert(!_name.isEmpty());

    msg->src()->putq(new UserRouterQueryResponse({msg->m_data.m_query_id, msg->m_data.m_query_name, _id, _name}, 0));
}

void MapServer::fill_opaque_message(UserRouterOpaqueRequest *msg)
{
	if (msg->m_data.m_target_id == 0 && (!msg->m_data.m_target_name.isEmpty()))
	{
		// get id from name
		for (EventProcessor *_mi : HandlerLocator::m_map_instances[m_id])
		{
			if (_mi == nullptr) 
				continue;

			MapInstance *mi = static_cast<MapInstance *>(_mi);
			for (MapClientSession *cl : mi->m_session_store)
			{
				if (cl->m_ent->name() == msg->m_data.m_target_name) 
				{
					msg->m_data.m_target_id = cl->m_ent->m_db_id;
					return;
				}
			}
		}
	}
}

void MapServer::route_opaque_message(UserRouterOpaqueRequest *msg) 
{
    MapInstance *ins = m_id_to_map_instance[msg->m_data.m_target_id];

    if (ins == nullptr) {
		qCDebug(logLogging) << "error routing message to map instance:" << msg->m_data.m_target_id;

		msg->src()->putq(new UserRouterOpaqueResponse({msg->m_data, UserRouterError::USER_OFFLINE}, 0));
        return;
    }

    InternalEvent *e = static_cast<InternalEvent *>(__route_unpack(msg->m_data.m_payload));
	e->session_token(msg->session_token());

    ins->putq(e, 0);
	msg->src()->putq(new UserRouterOpaqueResponse({msg->m_data, UserRouterError::OK}, msg->session_token()));
}

void MapServer::route_info_message(UserRouterInfoMessage *msg) 
{
    MapClientSession *sess = getMapClientSessionForEntityID(msg->m_data.m_target_id);

    if (sess == nullptr) {
        // TODO: handle message sent to invalid entity ID
        return;
    }

    sendInfoMessage(msg->m_data.m_info_channel, msg->m_data.m_message, *sess);
}

void MapServer::on_expect_client(ExpectMapClientRequest *ev)
{
    // TODO: handle contention while creating 2 characters with the same name from different clients
    // TODO: SELECT account_id from characters where name=ev->m_character_name
    const ExpectMapClientRequestData &request_data(ev->m_data);
    MapTemplate *tpl    = map_manager().get_template(request_data.m_map_name.toLower());
    if(nullptr==tpl)
    {
        qCDebug(logMapEvents) << "Returning response for base location...";
        ev->src()->putq(new ExpectMapClientResponse({1, 0, m_base_location}, ev->session_token()));
        return;
    }
    EventProcessor *instance = tpl->get_instance();
    // now we know which instance will handle this client, pass the event to it,
    // remember to shallow_copy to mark the event as still owned.
    instance->putq(ev->shallow_copy());
}

void MapServer::on_client_map_xfer(ClientMapXferMessage *ev)
{
    if(m_current_map_xfers.find(ev->m_data.m_session) == m_current_map_xfers.end())
    {
        m_current_map_xfers.insert(std::pair<uint64_t, uint8_t>(ev->m_data.m_session, ev->m_data.m_map_idx));
    }
    else
    {
        qCDebug(logMapXfers) << QString("Client session %1 attempted to request a second map transfer while having an existing transfer in progress").arg(ev->m_data.m_session);
    }
}

bool MapServer::session_has_xfer_in_progress(uint64_t session_token)
{
    return m_current_map_xfers.find(session_token) != m_current_map_xfers.end();
}

uint8_t MapServer::session_map_xfer_idx(uint64_t session_token)
{
    assert(session_has_xfer_in_progress(session_token));
    return m_current_map_xfers[session_token];
}

void MapServer::session_xfer_complete(uint64_t session_token)
{
    assert(session_has_xfer_in_progress(session_token));
    m_current_map_xfers.erase(session_token);
}

void MapServer::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void MapServer::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}
void MapServer::on_client_connected(ClientConnectedMessage *msg)
{
	EventProcessor *mi = HandlerLocator::getMapInstance_Handler(
		msg->m_data.m_server_id,
		msg->m_data.m_sub_server_id);

	qCritical() << "got client: " << msg->m_data.m_char_db_id << mi;
	m_id_to_map_instance[msg->m_data.m_char_db_id] = static_cast<MapInstance *>(mi);
}

void MapServer::on_client_disconnected(ClientDisconnectedMessage *msg)
{
    if (m_id_to_map_instance.count(msg->m_data.m_char_db_id))
        m_id_to_map_instance.erase(msg->m_data.m_char_db_id);
}
//! @}
