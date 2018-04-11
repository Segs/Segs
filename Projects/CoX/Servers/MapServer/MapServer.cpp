/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "MapServer.h"

#include "ServerManager.h"
#include "ConfigExtension.h"
#include "AdminServerInterface.h"
#include "MapClient.h"
#include "MapManager.h"
#include "MapServerData.h"
#include "MapTemplate.h"
#include "MapInstance.h"
#include "SEGSTimer.h"
#include "Settings.h"

#include <ace/Reactor.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <set>

// Template instantiation
template class ClientStore<MapClient>;
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


MapServer::MapServer() : d(new PrivateData)
{
    assert(g_GlobalMapServer==nullptr && "Only one GameServer instance per process allowed");
    g_GlobalMapServer = this;
}
MapServer::~MapServer()
{
    if(ACE_Reactor::instance())
    {
        ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK);
        delete m_endpoint;
    }
}
bool MapServer::Run()
{
    if(m_endpoint)
    {
        qWarning() << "Map server already running";
        return true;
    }
    if(!d->m_runtime_data.read_runtime_data("./data/bin/"))
    {
        return false;
    }
    assert(d->m_manager.num_templates()>0); // we have to have a world to run
    m_handler = d->m_manager.get_template(0)->get_instance();
    m_handler->set_server(this);


    m_endpoint = new MapLinkEndpoint(m_listen_point); //,this
    m_endpoint->set_downstream(m_handler);

    if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        qWarning() << "ACE_Reactor::register_handler";
        return false;
    }
    if (m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);
    return startup();
}
/**
 * @param  inipath Doc at RoamingServer::ReadConfig
 * @return bool (false means an error occurred )
 */
bool MapServer::ReadConfig()
{
    qInfo() << "Loading MapServer settings...";
    QSettings *config(Settings::getSettings());

    config->beginGroup("MapServer");
    if(m_endpoint)
    {
        //TODO: perform shutdown, and load config ?
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) MapServer already initialized and running\n") ));
        return true;
    }
    if(!config->contains(QStringLiteral("listen_addr")))
        qDebug() << "Config file is missing 'listen_addr' entry in MapServer group, will try to use default";
    if(!config->contains(QStringLiteral("location_addr")))
        qDebug() << "Config file is missing 'location_addr' entry in MapServer group, will try to use default";

    QString listen_addr = config->value("listen_addr","127.0.0.1:7003").toString();
    QString location_addr = config->value("location_addr","127.0.0.1:7003").toString();

    QString map_templates_dir = config->value("maps",".").toString();
    if(!parseAddress(listen_addr,m_listen_point))
    {
        qCritical() << "Badly formed IP address" << listen_addr;
        return false;
    }
    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }

    config->endGroup(); // MapServer

    return d->m_manager.load_templates(qPrintable(map_templates_dir));
}
bool MapServer::ShutDown(const QString &reason)
{
    if(!m_endpoint)
    {
        qWarning() << "MapServer has not been started yet.";
        return true;
    }
    // tell our handler to shut down too
    m_handler->putq(new SEGSEvent(0, nullptr));
    qWarning() << "Shutting down map server because :"<<reason;
    if (ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        delete m_endpoint;
        qCritical() << "ACE_Reactor::remove_handler failed";
        return false;
    }
    delete m_endpoint;
    GlobalTimerQueue::instance()->deactivate();

    return true;
}

const ACE_INET_Addr &MapServer::getAddress()
{
    return m_location;
}

EventProcessor *MapServer::event_target()
{
    return (EventProcessor *)m_handler;
}

GameServerInterface *MapServer::getGameInterface()
{
    return m_i_game;
}

MapManager &MapServer::map_manager()
{
    return d->m_manager;
}

MapServerData &MapServer::runtimeData()
{
    return d->m_runtime_data;
}

/**
* Processing according to MapServerStartup sequence diagram.
*/
bool MapServer::startup()
{
    //FIXME: global timer queue should be activated in some central place!
    GlobalTimerQueue::instance()->activate();
    return true;
}
