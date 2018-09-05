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
#include "MapTemplate.h"
#include "MapInstance.h"
#include "SEGSTimer.h"
#include "Settings.h"
#include "Servers/MessageBus.h"

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
        GameDataStore   m_runtime_data;
        MapManager      m_manager;
};


MapServer::MapServer(uint8_t id) : d(new PrivateData), m_id(id)
{
    assert(g_GlobalMapServer==nullptr && "Only one GameServer instance per process allowed");
    g_GlobalMapServer = this;
    HandlerLocator::setMap_Handler(id,this);
}

MapServer::~MapServer()
{

}

bool MapServer::Run()
{
    assert(m_owner_game_server_id != INVALID_GAME_SERVER_ID);

    if (!d->m_runtime_data.read_runtime_data(RUNTIME_DATA_PATH))
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
    d->m_runtime_data.m_player_fade_in = fade_in_variant.toFloat(&ok);
    if(!ok)
    {
        qCritical() << "Badly formed float for 'player_fade_in': " << fade_in_variant.toString();
        return false;
    }

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

GameDataStore &MapServer::runtimeData()
{
    return d->m_runtime_data;
}

void MapServer::sett_game_server_owner(uint8_t owner_id)
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
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
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
    if (m_current_map_xfers.find(ev->m_data.m_session) == m_current_map_xfers.end())
    {
        qCDebug(logMapEvents) << QString("Adding client transfer map index for client session %1 to map index %2").arg(ev->m_data.m_session).arg(ev->m_data.m_map_idx);
        m_current_map_xfers.insert(std::pair<uint64_t, uint8_t>(ev->m_data.m_session, ev->m_data.m_map_idx));
    }
    else
    {
        qCDebug(logMapEvents) << QString("Client session %1 attempted to request a second map transfer while having an existing transfer in progress").arg(ev->m_data.m_session);
    }
}

bool MapServer::session_has_xfer_in_progress(uint64_t session_token)
{
    return m_current_map_xfers.find(session_token) != m_current_map_xfers.end();
}

uint8_t MapServer::session_map_xfer_idx(uint64_t session_token)
{
    if (session_has_xfer_in_progress(session_token))
        return m_current_map_xfers[session_token];
}

void MapServer::session_xfer_complete(uint64_t session_token)
{
    if (session_has_xfer_in_progress(session_token))
        m_current_map_xfers.erase(session_token);
}

void MapServer::serialize_from(std::istream &is)
{
    assert(false);
}

void MapServer::serialize_to(std::ostream &is)
{
    assert(false);
}
//! @}
