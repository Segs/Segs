/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/EventProcessor.h"
#include "Common/Servers/ClientManager.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"

#include <unordered_set>
class CharacterClient;
class GameServer;
class SEGSTimer;
struct GameLink;
namespace SEGSEvents
{
class UpdateServer;
class UpdateCharacter;
class DeleteCharacter;
class MapServerAddrRequest;
class Timeout;

// internal events
class Idle;
class DisconnectRequest;
class ConnectRequest;
class UnknownEvent;
}
struct GameSession : public ClientSession
{
    enum eTravelDirection
    {
        EXITING_TO_MAP=0,
        EXITING_TO_LOGIN=1,
    };
    SEGSEvents::GameAccountResponseData m_game_account;
    uint32_t m_auth_account_id=0;
    uint32_t is_connected_to_map_server_id=0;
    uint32_t is_connected_to_map_instance_id=0;
    eTravelDirection m_direction;
    uint8_t m_access_level=0;

    uint32_t auth_id() const { return m_auth_account_id; }
    void reset()
    {
        *this = {}; // just use default constructed value
    }
    // those functions store temporariness state of the link in the lowest bit of the pointer
    void            set_temporary(bool v) { (intptr_t &)(m_link) = (intptr_t(m_link) & ~1) | intptr_t(v); }
    bool            is_temporary() const { return intptr_t(m_link) & 1; }
    GameLink *      link() { return (GameLink *)(intptr_t(m_link) & ~1); }
    /// \note setting the link does not preserver the state of the previous one.
    void            link(GameLink *l) { m_link = l; }

protected:
    GameLink *      m_link = nullptr;
};

class GameHandler final : public EventProcessor
{

    using sIds = std::unordered_set<uint32_t>;
    using SessionStore = ClientSessionStore<GameSession>;

    SessionStore    m_session_store;
    uint32_t        m_link_checker;
    uint32_t        m_service_status_timer;
    uint32_t        m_session_reaper_timer;
public:
                    IMPL_ID(GameHandler)
                    GameHandler();
                    ~GameHandler() override;
        void        set_server(GameServer *s) {m_server=s;}
        void        start();
protected:
        void        dispatch(SEGSEvents::Event *ev) override;

        //////////////////////////////////////////////////////////////////////////
        // Link events
        void        on_idle(SEGSEvents::Idle *ev);
        void        on_link_lost(SEGSEvents::Event *ev);
        void        on_disconnect(SEGSEvents::DisconnectRequest *ev);
        void        on_connection_request(SEGSEvents::ConnectRequest *ev);

        void        on_update_server(SEGSEvents::UpdateServer *ev);
        void        on_update_character(SEGSEvents::UpdateCharacter *ev);
        void        on_delete_character(SEGSEvents::DeleteCharacter *ev);
        void        on_map_req(SEGSEvents::MapServerAddrRequest *ev);
        void        on_unknown_link_event(SEGSEvents::UnknownEvent *ev);
    //////////////////////////////////////////////////////////////////////////
    // Server <-> Server events
        void        on_expect_client(SEGSEvents::ExpectClientRequest *ev);     // from AuthServer
        void        on_client_expected(SEGSEvents::ExpectMapClientResponse *ev); // from MapServer
        void        on_client_connected_to_other_server(SEGSEvents::ClientConnectedMessage *ev);
        void        on_client_disconnected_from_other_server(SEGSEvents::ClientDisconnectedMessage *ev);

    //////////////////////////////////////////////////////////////////////////
    // Internal events
        void        on_check_links();
        void        reap_stale_links();
        void        report_service_status();

        void        on_game_db_error(SEGSEvents::GameDbErrorMessage *ev);
        void        on_account_data(SEGSEvents::GameAccountResponse *ev);
        void        on_character_deleted(SEGSEvents::RemoveCharacterResponse *ev);
    //////////////////////////////////////////////////////////////////////////
        sIds        waiting_for_client; // this hash_set holds all client cookies we wait for
        GameServer *m_server;

        void        serialize_from(std::istream &is) override;
        void        serialize_to(std::ostream &is) override;
};
