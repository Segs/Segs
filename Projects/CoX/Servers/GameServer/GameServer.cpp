/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "GameServer.h"

#include "GameServerData.h"
#include "ServerManager.h"
#include "ConfigExtension.h"
#include "CharacterDatabase.h"
#include "AdminServerInterface.h"
#include "GameHandler.h"
#include "Common/CRUDP_Protocol/CRUDP_Protocol.h"
#include "Common/Servers/RoamingServer.h"

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

namespace {
    const constexpr int MaxAccountSlots=8;
}
GameServer *g_GlobalGameServer=nullptr;

class GameServer::PrivateData
{
public:
    QString                 m_serverName="";
    GameServerData          m_runtime_data;
    GameLinkEndpoint *      m_endpoint=nullptr;
    GameHandler *           m_handler=nullptr;
    GameLink *              m_game_link=nullptr;
    bool                    m_online=false;
    uint8_t                 m_id=0;
    uint16_t                m_current_players=0;
    int                     m_max_character_slots;
    uint16_t                m_max_players=0;
    uint8_t                 m_unk1=0;
    uint8_t                 m_unk2=0;
    ACE_INET_Addr           m_location; // this value is sent to the clients
    ACE_INET_Addr           m_listen_point; // the server binds here

    bool ShutDown(const QString &reason)
    {
        if(!m_endpoint)
        {
            qWarning() << "Server not running yet";
            return true;
        }
        m_online = false;
        // tell our handler to shut down too
        m_handler->putq(new SEGSEvent(0, nullptr));

        qWarning() << "Shutting down game server because : "<<reason;
        if (ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        {
            delete m_endpoint;
            qCritical() << "ACE_Reactor::remove_handler failed";
            return false;
        }
        delete m_endpoint;
        return true;

    }
};

GameServer::GameServer() : d(new PrivateData)
{
    assert(g_GlobalGameServer==nullptr && "Only one GameServer instance per process allowed");
    g_GlobalGameServer = this;
}

GameServer::~GameServer()
{
    if(ACE_Reactor::instance())
    {
        ACE_Reactor::instance()->remove_handler(d->m_endpoint,ACE_Event_Handler::READ_MASK);
        delete d->m_endpoint;
    }

}
bool GameServer::Run()
{
    if(d->m_endpoint)
    {
        qWarning() << "Game server already running";
        return true;
    }

    d->m_handler = new GameHandler;
    d->m_handler->set_server(this);
    d->m_handler->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
    d->m_handler->start();

    d->m_endpoint = new GameLinkEndpoint(d->m_listen_point); //,this
    d->m_endpoint->set_downstream(d->m_handler);

    if (ACE_Reactor::instance()->register_handler(d->m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
    if (d->m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);

    d->m_online = true;
    return true;
}
// later name will be used to read GameServer specific configuration
bool GameServer::ReadConfig(const QString &inipath)
{
    if(d->m_endpoint)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already initialized and running\n") ));
        return true;
    }
    if (!QFile::exists(inipath))
    {
        qCritical() << "Config file" << inipath <<"does not exist.";
        return false;
    }
    QSettings config(inipath,QSettings::IniFormat);
    config.beginGroup("GameServer");
    QString listen_addr = config.value("listen_addr","0.0.0.0:7002").toString();
    QString location_addr = config.value("location_addr","127.0.0.1:7002").toString();
    d->m_serverName = config.value("server_name","unnamed").toString();
    d->m_max_players = config.value("max_players",600).toUInt();
    d->m_max_character_slots = config.value("max_character_slots",MaxAccountSlots).toInt();
    if(!parseAddress(listen_addr,d->m_listen_point))
    {
        qCritical() << "Badly formed IP address" << listen_addr;
        return false;
    }
    if(!parseAddress(location_addr,d->m_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }

    d->m_current_players = 0;
    d->m_id = 1;
    d->m_unk1=d->m_unk2=0;
    d->m_online = false;
    //m_db = new GameServerDb("");
    return true;
}
bool GameServer::ShutDown(const QString &reason)
{
    return d->ShutDown(reason);
}

void GameServer::Online(bool s)
{
    d->m_online=s;
}

bool GameServer::Online()
{
    return d->m_online;
}

const ACE_INET_Addr &GameServer::getAddress()
{
    return d->m_location;
}
int GameServer::getAccessKeyForServer(const ServerHandle<IMapServer> &/*h_map*/)
{
    return 0;
}
bool GameServer::isMapServerReady(const ServerHandle<IMapServer> &/*h_map*/)
{
    return false;
}

QString GameServer::getName( )
{
    return d->m_serverName;
}

uint8_t GameServer::getId( )
{
    return d->m_id;
}

uint16_t GameServer::getCurrentPlayers( )
{
    return d->m_current_players;
}

uint16_t GameServer::getMaxPlayers()
{
    return d->m_max_players;
}

int GameServer::getMaxCharacterSlots() const
{
    return d->m_max_character_slots;
}

uint8_t GameServer::getUnkn1( )
{
    return d->m_unk1;
}

uint8_t GameServer::getUnkn2( )
{
    return d->m_unk2;
}

EventProcessor *GameServer::event_target()
{
    return (EventProcessor *)d->m_handler;
}
GameServerData &GameServer::runtimeData()
{
    return d->m_runtime_data;
}

Entity * GameServer::getEntityByName(const QString &name)
{
    Entity *pEnt = nullptr;
    EntityManager ref_em;

    // Iterate through all active entities and return entity by name
    for (auto *em : ref_em.m_live_entlist)
    {
        if (em->name() == name)
            return pEnt = em;
        else
            qDebug() << "Entity" << name << "does not exist.";
    }

    return pEnt;
}

Entity * GameServer::getEntityByIdx(const int32_t &idx)
{
    Entity *pEnt = nullptr;
    EntityManager ref_em;

    // Iterate through all active entities and return entity by idx
    for (auto *em : ref_em.m_live_entlist)
    {
        if (getIdx(*em) == idx)
            return pEnt = em;
        else
            qDebug() << "Entity ID" << idx << "does not exist.";
    }

    return pEnt;
}
