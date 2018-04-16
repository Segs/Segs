/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "GameServer.h"

#include "GameServerData.h"
#include "ConfigExtension.h"
#include "GameHandler.h"
#include "Servers/HandlerLocator.h"
#include "Common/CRUDP_Protocol/CRUDP_Protocol.h"
#include "Settings.h"

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
    const constexpr int MaxCharacterSlots=8;
    class GameLinkEndpoint : public ServerEndpoint
    {
    public:
        GameLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
        ~GameLinkEndpoint()=default;
    protected:
        CRUDLink *createLink(EventProcessor *down) override
        {
            return new GameLink(down,this);
        }
    };
}
class GameServer::PrivateData
{
public:
    QString                 m_serverName="";
    GameServerData          m_runtime_data;
    GameLinkEndpoint *      m_endpoint=nullptr;
    GameHandler *           m_handler=nullptr;
    GameLink *              m_game_link=nullptr;
    bool                    m_online=false;
    uint8_t                 m_id=1;
    uint16_t                m_current_players=0;
    int                     m_max_character_slots;
    uint16_t                m_max_players=0;
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
        m_handler->putq(new SEGSEvent(SEGS_EventTypes::evFinish, nullptr));

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
void GameServer::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case Internal_EventTypes::evReloadConfig:
            ReadConfigAndRestart();
            break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}

GameServer::GameServer(int id) : d(new PrivateData)
{
    d->m_handler = new GameHandler;
    d->m_handler->set_server(this);
    d->m_handler->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
    d->m_handler->start();
    d->m_id = id;
    HandlerLocator::setGame_Handler(d->m_id,d->m_handler);
}

GameServer::~GameServer()
{
    if(ACE_Reactor::instance())
    {
        ACE_Reactor::instance()->remove_handler(d->m_endpoint,ACE_Event_Handler::READ_MASK);
        delete d->m_endpoint;
    }
}
// later name will be used to read GameServer specific configuration
bool GameServer::ReadConfigAndRestart()
{
    static GameServerReconfigured reconfigured_msg;
    if(d->m_endpoint) // TODO: consider properly closing all open sessions ?
        delete d->m_endpoint;
    qInfo() << "Loading GameServer settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("GameServer"));
    if(!config.contains(QStringLiteral("listen_addr")))
        qWarning() << "Config file is missing 'listen_addr' entry in GameServer group, will try to use default";
    if(!config.contains(QStringLiteral("location_addr")))
        qWarning() << "Config file is missing 'location_addr' entry in GameServer group, will try to use default";

    QString listen_addr = config.value(QStringLiteral("listen_addr"),"127.0.0.1:7002").toString();
    QString location_addr = config.value(QStringLiteral("location_addr"),"127.0.0.1:7002").toString();

    d->m_serverName = config.value(QStringLiteral("server_name"),"unnamed").toString();
    d->m_max_players = config.value(QStringLiteral("max_players"),600).toUInt();
    d->m_max_character_slots = config.value(QStringLiteral("max_character_slots"),MaxCharacterSlots).toInt();
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
    d->m_online = false;
    //m_db = new GameServerDb("");

    config.endGroup(); // GameServer
    // Begin server spin up

    d->m_endpoint = new GameLinkEndpoint(d->m_listen_point); //,this
    d->m_endpoint->set_downstream(d->m_handler);

    if (ACE_Reactor::instance()->register_handler(d->m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
    if (d->m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);

    qInfo() << "  configuration loaded and server started";
    d->m_online = true;
    d->m_handler->putq(reconfigured_msg.shallow_copy());
    return true;
}
bool GameServer::ShutDown(const QString &reason)
{
    return d->ShutDown(reason);
}

const ACE_INET_Addr &GameServer::getAddress()
{
    return d->m_location;
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

EventProcessor *GameServer::event_target()
{
    return (EventProcessor *)d->m_handler;
}
GameServerData &GameServer::runtimeData()
{
    return d->m_runtime_data;
}
