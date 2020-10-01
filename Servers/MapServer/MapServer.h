/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/ServerEndpoint.h"
#include "Common/GameData/map_definitions.h"
#include "GameServer/GameServer.h"
//#include "Entity.h"
#include "MapLink.h"

#include <memory>

class Net;
class MapServerEndpoint;
struct MapClientSession;
class MapInstance;
class GameDataStore;
class MapManager;
namespace SEGSEvents
{
struct ExpectMapClientRequest;
struct ClientMapXferMessage;
struct UserRouterInfoMessage;
struct UserRouterOpaqueRequest;
struct UserRouterOpaqueResponse;
struct UserRouterQueryRequest;
struct UserRouterQueryResponse;
struct TeamUpdatedMessage;
}

static constexpr uint8_t INVALID_GAME_SERVER_ID = 255;
static constexpr char RUNTIME_DATA_PATH[] = "./data/";

class MapServer final : public EventProcessor
{
        class PrivateData;
public:
                                IMPL_ID(MapServer)
                                MapServer(uint8_t id);
                                ~MapServer() override;

        bool                    ReadConfigAndRestart();

        void                    per_thread_shutdown() override;
        MapManager &            map_manager();
        void                    set_game_server_owner(uint8_t owner_id);
        bool                    session_has_xfer_in_progress(uint64_t session_token);
        MapXferData &           session_map_xfer_idx(uint64_t session_token);
        void                    session_xfer_complete(uint64_t session_token);
private:
        bool                    Run();
        // EventProcessor interface
        void                    dispatch(SEGSEvents::Event *ev) override;
        void                    on_expect_client(SEGSEvents::ExpectMapClientRequest *ev);
        void                    on_client_map_xfer(SEGSEvents::ClientMapXferMessage *ev);
        void                    serialize_from(std::istream &is) override;
        void                    serialize_to(std::ostream &is) override;

        std::unique_ptr<PrivateData> d;

        uint8_t                             m_id = 1;
        uint8_t                             m_owner_game_server_id = INVALID_GAME_SERVER_ID;
        ACE_INET_Addr                       m_base_location; //! this is the base map instance address
        ACE_INET_Addr                       m_base_listen_point; //! this is used as a base map listening endpoint
        std::map<uint64_t, MapXferData>     m_current_map_transfers;    // Current map transfers in progress on this map server.
};

extern MapServer *g_GlobalMapServer;
