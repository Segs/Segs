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
#include "GameServer/GameServer.h"
//#include "Entity.h"
#include "MapLink.h"

#include <memory>

class Net;
class MapServerEndpoint;
struct MapClientSession;
class MapInstance;
class GameServerInterface;
class MapServerData;
class MapManager;

class MapLinkEndpoint : public ServerEndpoint
{
public:
    MapLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
    ~MapLinkEndpoint()=default;
protected:
    CRUDLink *createLink(EventProcessor *down) override
    {
        return new MapLink(down,this);
    }
};
class MapServer : public IMapServer
{
        class PrivateData;
public:
                                MapServer(void);
                                ~MapServer(void) override;

        bool                    Run(void) override;
        bool                    ReadConfig() override;

        bool                    ShutDown(const QString &reason="No particular reason") override;
        const ACE_INET_Addr &   getAddress() override;
        EventProcessor *        event_target() override;
        MapManager &            map_manager();
        MapServerData &         runtimeData();
private:
        bool                    startup(); // MapServerStartup sequence diagram entry point.
protected:
        std::unique_ptr<PrivateData> d;

        uint8_t                 m_id = 1;

        QString                 m_serverName;
        ACE_INET_Addr           m_location; //! this value is sent to the clients
        ACE_INET_Addr           m_listen_point; //! this is used as a listening endpoint
        MapLinkEndpoint *       m_endpoint = nullptr;
        MapInstance *           m_handler = nullptr;
};
extern MapServer *g_GlobalMapServer;
