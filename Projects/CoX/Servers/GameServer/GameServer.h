/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once

#include <string>

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "CRUDP_Protocol.h"
#include "RoamingServer.h"
#include "ServerEndpoint.h"
#include "GameEvents.h"

class GameHandler;
class pktCS_ServerUpdate;
class CharacterDatabase;
typedef std::list< ServerHandle<MapServer> > lMapServerHandles;
class GameServer : public IGameServer
{
    typedef ServerEndpoint<GameLink> GameLinkEndpoint;
public:
                                ~GameServer(void);
                                GameServer(void);
        bool                    ReadConfig(const std::string &configpath); // later name will be used to read GameServer specific configuration
        bool                    Run(void);
        bool                    ShutDown(const std::string &reason="No particular reason");
        void                    Online(bool s ) {m_online=s;}
        bool                    Online(void) { return m_online;}
        const ACE_INET_Addr &	getAddress() {return m_location;}

        // World-cluster management interface
        int                     getAccessKeyForServer(const ServerHandle<IMapServer> &h_map);
        bool                    isMapServerReady(const ServerHandle<IMapServer> &h_map);

        ServerHandle<IMapServer> GetMapHandle(const std::string &mapname);


        std::string             getName(void);
        uint8_t                 getId(void);
        uint16_t                getCurrentPlayers(void);
        uint16_t                getMaxPlayers();
        uint8_t                 getUnkn1(void);
        uint8_t                 getUnkn2(void);
        CharacterDatabase *     getDb();

        int                     createLinkedAccount(uint64_t auth_account_id,const std::string &username); // Part of exposed db interface.
        EventProcessor *        event_target() {return (EventProcessor *)m_handler;}

protected:
        uint32_t                GetClientCookie(const ACE_INET_Addr &client_addr); // returns a cookie that will identify user to the gameserver
        lMapServerHandles       GetMapsHandling(const std::string &mapname);
        bool                    m_online;
        uint8_t                 m_id;
        uint16_t                m_current_players;
        uint16_t                m_max_players;
        uint8_t                 m_unk1,m_unk2;
        std::string             m_serverName;
        ACE_INET_Addr           m_location; // this value is sent to the clients
        ACE_INET_Addr           m_listen_point; // the server binds here
        GameLinkEndpoint *      m_endpoint;
        GameHandler *           m_handler;
};
