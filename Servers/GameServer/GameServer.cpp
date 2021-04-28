/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameServer Projects/CoX/Servers/GameServer
 * @{
 */

#include "GameServer.h"
#include "Messages/Game/GameEvents.h"

#include "FriendshipService/FriendHandler.h"
#include "Components/ConfigExtension.h"
#include "GameHandler.h"
#include "Servers/HandlerLocator.h"
#include "Common/Servers/ServerEndpoint.h"
#include "EmailService/EmailHandler.h"
#include "TeamService/TeamHandler.h"
#include "Components/Settings.h"

#include "Messages/TeamService/TeamEvents.h"
#include "Messages/UserRouterService/UserRouterEvents.h"

#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/Message_Queue.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

using namespace SEGSEvents;
namespace
{
    const constexpr int MaxCharacterSlots=8;
    class GameLinkEndpoint final : public ServerEndpoint
    {
        public:
            GameLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
            ~GameLinkEndpoint() override = default ;
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
    std::unique_ptr<FriendHandler> m_friendship_service;
    std::unique_ptr<EmailHandler> m_email_service;
    std::unique_ptr<TeamHandler> m_team_service;
    ACE_INET_Addr           m_location; // this value is sent to the clients
    ACE_INET_Addr           m_listen_point; // the server binds here
    GameLinkEndpoint *      m_endpoint=nullptr;
    GameHandler *           m_handler=nullptr;
    GameLink *              m_game_link=nullptr;
    bool                    m_online=false;
    uint8_t                 m_id=1;
    uint16_t                m_current_players = 0;
    int                     m_max_character_slots;
    uint16_t                m_max_players = 0;

    void ShutDown() const
    {
        // tell our handler to shut down
        shutdown_event_processor_and_wait(m_handler);
        // tell our child services to close too
        shutdown_event_processor_and_wait(m_email_service.get());
        shutdown_event_processor_and_wait(m_friendship_service.get());
        shutdown_event_processor_and_wait(m_team_service.get());

    }
};

void GameServer::dispatch(Event *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case evReloadConfigMessage:
            ReadConfigAndRestart();
            break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}

void GameServer::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void GameServer::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}

extern void register_TeamServiceEvents();

GameServer::GameServer(int id) : d(new PrivateData)
{
    register_TeamServiceEvents();

    d->m_handler = new GameHandler;
    d->m_handler->set_server(this);
    d->m_handler->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
    d->m_handler->start();
    d->m_id = id;

    d->m_email_service = std::make_unique<EmailHandler>(id);
    d->m_email_service->activate();
    d->m_email_service->set_db_handler(d->m_id);

    d->m_friendship_service = std::make_unique<FriendHandler>(id);
    d->m_friendship_service->activate();

    d->m_team_service = std::make_unique<TeamHandler>(id);
    d->m_team_service->activate();

    HandlerLocator::setGame_Handler(d->m_id,d->m_handler);
}

GameServer::~GameServer()
{
    d->ShutDown();
    delete d->m_endpoint;
}

// later name will be used to read GameServer specific configuration
bool GameServer::ReadConfigAndRestart()
{
    static ServerReconfigured reconfigured_msg;
    // TODO: consider properly closing all open sessions ?
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

    if(ACE_Reactor::instance()->register_handler(d->m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
    if(d->m_endpoint->open() == -1) // will register notifications with current reactor
        ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);

    qInfo() << "Configurations loaded";
    d->m_online = true;
    d->m_handler->putq(reconfigured_msg.shallow_copy());
    return true;
}

const ACE_INET_Addr &GameServer::getAddress()
{
    return d->m_location;
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

int GameServer::handle_close(ACE_HANDLE /*handle*/, ACE_Reactor_Mask /*close_mask*/)
{
    // after evfinish some other messages could have been added to the queue, release them
    d->ShutDown();
    assert(d->m_handler->msg_queue()->is_empty());
    qWarning() << "Shutting down game server";
    return 0;
}

//! @}
