/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "MapServer.h"

#include "ServerManager.h"
#include "InterfaceManager.h"
#include "ConfigExtension.h"
#include "AdminServerInterface.h"
#include "MapClient.h"
#include "MapTemplate.h"
#include "MapInstance.h"
#include "Entity.h"
#include "SEGSTimer.h"

#include <ace/Log_Msg.h>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>
// Template instantiation
template class ClientStore<MapClient>;

MapServer::MapServer() : m_id(0),
    m_online(false),
    m_endpoint(nullptr)
{

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
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already running\n") ));
        return true;
    }

    assert(m_manager.num_templates()>0); // we have to have a world to run
    m_handler = m_manager.get_template(0)->get_instance();
    m_handler->set_server(this);


    m_endpoint = new MapLinkEndpoint(m_listen_point); //,this
    m_endpoint->set_downstream(m_handler);

    if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "ACE_Reactor::register_handler"),false);
    if (m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);
    return startup();
}
/**
 * @param  inipath Doc at RoamingServer::ReadConfig
 * @return bool (false means an error occurred )
 */
bool MapServer::ReadConfig(const std::string &inipath)
{
    if (!QFile::exists(inipath.c_str()))
    {
        qCritical() << "Config file" << inipath.c_str() <<"does not exist.";
        return false;
    }
    QSettings config(inipath.c_str(),QSettings::IniFormat);
    config.beginGroup("MapServer");

    if(m_endpoint)
    {
        //TODO: perform shutdown, and load config ?
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) MapServer already initialized and running\n") ));
        return true;
    }
    //TODO: this should read a properly nested MapServer/RoamingServer block, instead of reading ini-'global' [RoamingServer]
    if(!RoamingServer::ReadConfig(inipath)) // try to read control channel configuration
        return false;

    QString listen_addr = config.value("listen_addr","0.0.0.0:7003").toString();
    QString location_addr = config.value("location_addr","127.0.0.1:7003").toString();
    QString map_templates_dir = config.value("maps",".").toString();
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

    m_online = false;
    return m_manager.load_templates(qPrintable(map_templates_dir));
}
bool MapServer::ShutDown(const std::string &reason)
{
    if(!m_endpoint)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Server not running yet\n") ));
        return true;
    }
    m_online = false;
    ACE_DEBUG((LM_WARNING,ACE_TEXT ("(%P|%t) Shutting down map server because : %s\n"), reason.c_str()));
    if (ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        delete m_endpoint;
        ACE_ERROR_RETURN ((LM_ERROR, "ACE_Reactor::remove_handler"),false);
    }
    delete m_endpoint;
    return true;
}

/**
* Processing according to MapServerStartup sequence diagram.
*/
bool MapServer::startup()
{
    //FIXME: global timer queue should be activated in some central place!
    GlobalTimerQueue::instance()->activate();
    return true;
    AuthServerInterface *i_auth;
    AdminServerInterface *i_admin;
    //GameServerInterface *i_game;

    ServerHandle<IMapServer> h_me(m_listen_point,m_id);

    i_auth = getAuthServer(); // connect to AuthServer
    i_admin= InterfaceManager::instance()->get(i_auth->AuthenticateMapServer(h_me,MAPSERVER_VERSION,"some_password"));// Authenticate
    if(!i_admin)
    {
        //Auth failure, Errors could be passed here by async AuthServer->MapServer calls
        return false;
    }

    m_online = true;
#if 0
    m_i_game=InterfaceManager::instance()->get(i_admin->RegisterMapServer(h_me));
    if(!m_i_game)
    {
        //Couldn't register self with mapserver
        return false;
    }
    return m_i_game->MapServerReady(h_me); // inform game server that we are ready.
#else
    return true;
#endif
}
