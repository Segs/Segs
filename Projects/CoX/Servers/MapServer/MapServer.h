/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "Common/Servers/ServerEndpoint.h"
#include "GameServer/GameServer.h"
//#include "Entity.h"
#include "MapLink.h"

#include <memory>

class Net;
class MapServerEndpoint;
struct MapClientSession;
class MapInstance;
class MapServerData;
class MapManager;

class MapServer : public EventProcessor
{
        class PrivateData;
public:
                                MapServer(uint8_t id);
                                ~MapServer(void) override;

        bool                    ReadConfigAndRestart();

        bool                    ShutDown(const QString &reason="No particular reason");
        MapManager &            map_manager();
        MapServerData &         runtimeData();
        void                    sett_game_server_owner(uint8_t owner_id);
private:
        bool                    Run(void);
        // EventProcessor interface
        void                    dispatch(SEGSEvent *ev) override;
        void                    on_expect_client(struct ExpectMapClientRequest *ev);

        std::unique_ptr<PrivateData> d;

        uint8_t                 m_id = 1;
        uint8_t                 m_owner_game_server_id = 255;
        QString                 m_serverName;
        ACE_INET_Addr           m_base_location; //! this is the base map instance address
        ACE_INET_Addr           m_base_listen_point; //! this is used as a base map listening endpoint
        MapInstance *           m_handler = nullptr;
};
extern MapServer *g_GlobalMapServer;
