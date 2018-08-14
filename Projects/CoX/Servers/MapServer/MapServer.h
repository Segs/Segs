/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
namespace SEGSEvents
{
struct ExpectMapClientRequest;
}

static constexpr uint8_t INVALID_GAME_SERVER_ID = 255;
static constexpr char RUNTIME_DATA_PATH[] = "./data/bin/";

class MapServer final : public EventProcessor
{
        class PrivateData;
public:
                                MapServer(uint8_t id);
                                ~MapServer() override;

        bool                    ReadConfigAndRestart();

        void                    per_thread_shutdown() override;
        MapManager &            map_manager();
        MapServerData &         runtimeData();
        void                    sett_game_server_owner(uint8_t owner_id);
private:
        bool                    Run();
        // EventProcessor interface
        void                    dispatch(SEGSEvents::Event *ev) override;
        void                    on_expect_client(SEGSEvents::ExpectMapClientRequest *ev);

        std::unique_ptr<PrivateData> d;

        uint8_t                 m_id = 1;
        uint8_t                 m_owner_game_server_id = INVALID_GAME_SERVER_ID;
        ACE_INET_Addr           m_base_location; //! this is the base map instance address
        ACE_INET_Addr           m_base_listen_point; //! this is used as a base map listening endpoint
};

extern MapServer *g_GlobalMapServer;
