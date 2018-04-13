#pragma once
#include "ClientManager.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "EventProcessor.h"
#include "InternalEvents.h"
#include "Common/Servers/MessageBusEndpoint.h"
#include "GameDatabase/GameAccountData.h"
#include "AuthDatabase/AccountData.h"

#include <map>
#include <unordered_set>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
class CharacterClient;
class GameServer;
class SEGSTimer;

struct GameSession
{
    enum eClientState
    {
        CLIENT_DISCONNECTED = 0,
        CLIENT_EXPECTED,
        NOT_LOGGED_IN,
        LOGGED_IN,
        CLIENT_CONNECTED
    };
    GameLink *      m_link = nullptr;
    AuthAccountData m_account;
    GameAccountData m_game_account;
    uint32_t        is_connected_to_map_server_id   = 0;
    uint32_t        is_connected_to_map_instance_id = 0;
    eClientState    m_state;
    uint32_t auth_id() const { return m_account.m_acc_server_acc_id; }
    void reset()
    {
        *this = {}; // just use default constructed value
    }
};
class GameHandler : public EventProcessor
{
    using sIds = std::unordered_set<uint32_t>;
    using SessionStore = ClientSessionStore<GameSession>;

    SessionStore m_session_store;
    std::unique_ptr<SEGSTimer> m_link_checker;

public:
                GameHandler();
                ~GameHandler();
    void        set_server(GameServer *s) {m_server=s;}
    void        start();
protected:
        void    dispatch(SEGSEvent *ev);

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
    void        on_timeout(TimerEvent *ev);

    //////////////////////////////////////////////////////////////////////////
    sIds        waiting_for_client; // this hash_set holds all client cookies we wait for
    GameServer *m_server;

};

