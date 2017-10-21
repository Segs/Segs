/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "ConfigExtension.h"
#include "GameServer.h"
#include "CharacterDatabase.h"
#include "AdminServerInterface.h"
#include "GameHandler.h"
#include "Common/GameData/WorldData.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

GameServer::GameServer() :
    m_online(false),
    m_id(0),
    m_current_players(0),
    m_max_players(0),
    m_unk1(0),
    m_unk2(0),
    m_serverName(""),
    m_endpoint(NULL)
{
}

GameServer::~GameServer()
{
    if(ACE_Reactor::instance())
    {
        ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK);
        delete m_endpoint;
    }

}
bool GameServer::Run()
{
    if(m_endpoint)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already running\n") ));
        return true;
    }
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading game data from ./data/bin/ folder\n") ));
    WorldData::instance()->read_costumes("./data/bin/");
    WorldData::instance()->read_colors("./data/bin/");
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) All game data read\n") ));
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Filling hashes .. ") ));
    WorldData::instance()->fill_hashes();
    ACE_DEBUG((LM_WARNING,ACE_TEXT("Hashes filled\n") ));

    m_handler = new GameHandler;
    m_handler->set_server(this);
    m_handler->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
    m_handler->start();

    m_endpoint = new GameLinkEndpoint(m_listen_point); //,this
    m_endpoint->set_downstream(m_handler);

    if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
    if (m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);

    m_online = true;
    return true;
}
bool GameServer::ReadConfig(const std::string &inipath)
{
    if(m_endpoint)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already initialized and running\n") ));
        return true;
    }
    if (!QFile::exists(inipath.c_str()))
    {
        qCritical() << "Config file" << inipath.c_str() <<"does not exist.";
        return false;
    }
    QSettings config(inipath.c_str(),QSettings::IniFormat);
    config.beginGroup("GameServer");
    QString listen_addr = config.value("listen_addr","0.0.0.0:7002").toString();
    QString location_addr = config.value("location_addr","127.0.0.1:7002").toString();
    m_serverName = config.value("server_name","unnamed").toString().toStdString();
    m_max_players = config.value("max_players",600).toUInt();
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

    m_current_players = 0;
    m_id = 1;
    m_unk1=m_unk2=0;
    m_online = false;
    //m_db = new GameServerDb("");
    return true;
}
bool GameServer::ShutDown(const std::string &reason)
{
    if(!m_endpoint)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Server not running yet\n") ));
        return true;
    }
    m_online = false;
    ACE_DEBUG((LM_WARNING,ACE_TEXT ("(%P|%t) Shutting down game server because : %s\n"), reason.c_str()));
    if (ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
    {
        delete m_endpoint;
        ACE_ERROR_RETURN ((LM_ERROR, "ACE_Reactor::remove_handler"),false);
    }
    delete m_endpoint;
    return true;
}
int GameServer::getAccessKeyForServer(const ServerHandle<IMapServer> &/*h_map*/)
{
    return 0;
}
bool GameServer::isMapServerReady(const ServerHandle<IMapServer> &/*h_map*/)
{
    return false;
}

std::string GameServer::getName( )
{
    return m_serverName;
}

uint8_t GameServer::getId( )
{
    return m_id;
}

uint16_t GameServer::getCurrentPlayers( )
{
    return m_current_players;
}

uint16_t GameServer::getMaxPlayers()
{
    return m_max_players;
}

uint8_t GameServer::getUnkn1( )
{
    return m_unk1;
}

uint8_t GameServer::getUnkn2( )
{
    return m_unk2;
}

//

