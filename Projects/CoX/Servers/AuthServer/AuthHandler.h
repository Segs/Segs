#pragma once
#include "Servers/InternalEvents.h"
#include "AuthDatabase/AccountData.h"
#include "AuthProtocol/AuthLink.h"
#include "AuthProtocol/AuthEvents.h"
#include "Servers/MessageBusEndpoint.h"
#include "Servers/ClientManager.h"
#include "EventProcessor.h"
#include "AdminServer/AccountInfo.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Addr.h>
#include <unordered_map>
#include <map>

class AuthServer;
class SEGSTimer;
struct RetrieveAccountResponse;
struct ValidatePasswordResponse;

enum eAuthError
{
    AUTH_SERVER_OFFLINE = -1,
    //AUTH_OK = 0,
    AUTH_ACCOUNT_BLOCKED = 0,
    AUTH_DATABASE_ERROR = 1,
    AUTH_INVALID_ACCOUNT = 2, // no client side message
    AUTH_WRONG_LOGINPASS = 3,
    AUTH_ACCOUNT_SYNC_FAIL = 4,
    // 5- SSN not available
    // 6 - no server list
    AUTH_ALREADY_LOGGEDIN = 7,
    AUTH_UNAVAILABLE_SERVER = 8,
    AUTH_KICKED_FROM_GAME = 11,
    AUTH_SELECTED_SERVER_FULL = 15,
    AUTH_CHANGE_PASSWORD = 17,
    AUTH_ACCOUNT_TIME_EXPIRED = 18,
    AUTH_NO_PAID_TIME_REMAINS = 19,
    AUTH_UNKN_ERROR
}; // this is a public type so other servers can pass us valid errors

struct AuthSession
{
    enum eClientState
    {
        CLIENT_DISCONNECTED=0,
        CLIENT_EXPECTED,
        NOT_LOGGED_IN,
        LOGGED_IN,
        CLIENT_CONNECTED
    };
    AuthLink *m_link = nullptr;
    std::unique_ptr<AuthAccountData> m_auth_data;
    uint32_t m_auth_id=0;
    eClientState m_state = NOT_LOGGED_IN;
    uint32_t is_connected_to_game_server_id=0;
    uint32_t auth_id() const { return m_auth_id; }
};
class AuthHandler : public EventProcessor
{
    using SessionStore = ClientSessionStore<AuthSession>;
protected:
    static uint64_t s_last_session_id;
    MessageBusEndpoint m_message_bus_endpoint;
    SessionStore m_sessions;
    AuthServer *m_authserv = nullptr;

    bool        isClientConnectedAnywhere(uint32_t client_id);
    void        reap_stale_links();

    //////////////////////////////////////////////////////////////////////////
    // internal events
    void        on_timeout(TimerEvent *ev);
    //////////////////////////////////////////////////////////////////////////
    // function that send messages into the link
    void        auth_error(EventProcessor *lnk,uint32_t code);
    //////////////////////////////////////////////////////////////////////////
    // incoming link level event handlers
    void        on_connect(ConnectEvent *ev);
    void        on_disconnect(DisconnectEvent *ev);
    void        on_login( LoginRequest *ev );
    void        on_server_list_request( ServerListRequest *ev );
    void        on_server_selected(ServerSelectRequest *ev);
    //////////////////////////////////////////////////////////////////////////
    // Server <-> server event handlers
    void        on_client_expected(ExpectClientResponse *ev);
    void        on_client_connected_to_other_server(ClientConnectedMessage *ev);
    void        on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev);

    void        dispatch(SEGSEvent *ev) override;
public:
                AuthHandler(AuthServer *our_server);
};

