/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once

#include "Common/Servers/MapServerInterface.h"
#include "Common/Servers/ServerEndpoint.h"
#include "MapLink.h"
#include "MapManager.h"

#include <string>

class Net;
class MapServerEndpoint;
class MapClient;
class MapInstance;
class GameServerInterface;

class MapLinkEndpoint : public ServerEndpoint
{
public:
    MapLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
    ~MapLinkEndpoint()=default;
protected:
    ILink *createLink(EventProcessor *down) override
    {
        return new MapLink(down,this);
    }
};
class MapServer : public IMapServer
{
static const int                MAPSERVER_VERSION=1;

public:
                                MapServer(void);
virtual                         ~MapServer(void);

virtual bool                    Run(void);
virtual bool                    ReadConfig(const std::string &name);

        bool                    ShutDown(const std::string &reason="No particular reason");
        void                    Online(bool s ) {m_online=s;}
        bool                    Online(void) { return m_online;}
        const ACE_INET_Addr &   getAddress() {return m_location;}
        EventProcessor *        event_target() {return (EventProcessor *)m_handler;}
        GameServerInterface *   getGameInterface(){return m_i_game;}
        MapManger &             map_manager() {return m_manager;}
private:
        bool                    startup(); // MapServerStartup sequence diagram entry point.
protected:
        uint8_t                 m_id;
        bool                    m_online;
        MapManger               m_manager;
        GameServerInterface *   m_i_game;// GameServer access proxy object

        std::string             m_serverName;

        ACE_INET_Addr           m_location; //! this value is sent to the clients
        ACE_INET_Addr           m_listen_point; //! this is used as a listening endpoint
        MapLinkEndpoint *       m_endpoint;
        MapInstance *           m_handler;
};
