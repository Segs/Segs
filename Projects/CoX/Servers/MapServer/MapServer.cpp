/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapServer.h"

#include "Servers/HandlerLocator.h"
#include "ConfigExtension.h"
#include "MapManager.h"
#include "MapServerData.h"
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
        MapServerData   m_runtime_data;
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
    assert(m_owner_game_server_id!=255);
    if(!d->m_runtime_data.read_runtime_data("./data/bin/"))
    {
        return false;
    }
    assert(d->m_manager.num_templates()>0); // we have to have a world to run

    qInfo() << "Server running... awaiting client connections."; // best place for this?
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

    config.endGroup(); // MapServer

    if(!d->m_manager.load_templates(map_templates_dir,m_owner_game_server_id,m_id,{m_base_listen_point,m_base_location}))
    {
        postGlobalEvent(new ServiceStatusMessage({ QString("MapServer: Cannot load map templates from %1").arg(map_templates_dir),-1 }));
        return false;
    }
    return Run();
}

bool MapServer::ShutDown()
{
    qWarning() << "Shutting down map server";
    // tell all instances to shut down too
    d->m_manager.shut_down_all();
    putq(SEGSEvent::s_ev_finish.shallow_copy());
    return true;
}

MapManager &MapServer::map_manager()
{
    return d->m_manager;
}

MapServerData &MapServer::runtimeData()
{
    return d->m_runtime_data;
}

void MapServer::sett_game_server_owner(uint8_t owner_id)
{
    m_owner_game_server_id = owner_id;
}

void MapServer::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case Internal_EventTypes::evReloadConfig:
            ReadConfigAndRestart();
            break;
        case Internal_EventTypes::evExpectMapClientRequest:
            on_expect_client(static_cast<ExpectMapClientRequest *>(ev));
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
    MapTemplate *tpl    = map_manager().get_template(request_data.m_map_name);
    if(nullptr==tpl)
    {
        
        HandlerLocator::getGame_Handler(m_owner_game_server_id)->putq(
            new ExpectMapClientResponse({1, 0, m_base_location}, ev->session_token()));
        return;
    }
    EventProcessor *instance = tpl->get_instance();
    // now we know which instance will handle this client, pass the event to it,
    // remember to shallow_copy to mark the event as still owned.
    instance->putq(ev->shallow_copy());
}

//! @}
