#pragma once
#include "GameLink.h"
#include "GameEvents.h"
#include "EventProcessor.h"
#include "Common/Servers/ClientManager.h"
#include "GameDatabase/GameDBSyncEvents.h"

#include <unordered_set>
class CharacterClient;
class GameServer;
class SEGSTimer;

struct GameSession
{
    enum eTravelDirection
    {
        EXITING_TO_MAP=0,
        EXITING_TO_LOGIN=1,
    };
    GameAccountResponseData m_game_account;
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

    SessionStore m_session_store;
    std::unique_ptr<SEGSTimer> m_link_checker;
    std::unique_ptr<SEGSTimer> m_service_status_timer;

public:
                    GameHandler();
                    ~GameHandler();
        void        set_server(GameServer *s) {m_server=s;}
        void        start();
protected:
        void        dispatch(SEGSEvent *ev) override;

        //////////////////////////////////////////////////////////////////////////
        // Link events
        void        on_idle(IdleEvent *ev);
        void        on_link_lost(SEGSEvent *ev);
        void        on_disconnect(DisconnectRequest *ev);
        void        on_connection_request(ConnectRequest *ev);

        void        on_update_server(UpdateServer *ev);
        void        on_update_character(UpdateCharacter *ev);
        void        on_delete_character(DeleteCharacter *ev);
        void        on_map_req(MapServerAddrRequest *ev);
        void        on_unknown_link_event(GameUnknownRequest *ev);
    //////////////////////////////////////////////////////////////////////////
    // Server <-> Server events
        void        on_expect_client(ExpectClientRequest *ev);     // from AuthServer
        void        on_client_expected(ExpectMapClientResponse *ev); // from MapServer
        void        on_client_connected_to_other_server(ClientConnectedMessage *ev);
        void        on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev);

    //////////////////////////////////////////////////////////////////////////
    // Internal events
        void        on_check_links();
        void        reap_stale_links();
        void        report_service_status();
        void        on_timeout(TimerEvent *ev);

        void        on_game_db_error(GameDbErrorMessage *ev);
        void        on_account_data(GameAccountResponse *ev);
        void        on_character_deleted(RemoveCharacterResponse *ev);
    //////////////////////////////////////////////////////////////////////////
        sIds        waiting_for_client; // this hash_set holds all client cookies we wait for
        GameServer *m_server;
};
