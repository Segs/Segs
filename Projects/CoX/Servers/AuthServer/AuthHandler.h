#pragma once
#include "Servers/InternalEvents.h"
#include "AuthProtocol/AuthLink.h"
#include "AuthProtocol/AuthEvents.h"
#include "Servers/MessageBusEndpoint.h"
#include "EventProcessor.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Addr.h>
#include <unordered_map>
#include <map>

class AuthServer;
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

class AuthHandler : public EventProcessor
{
protected:
    using MTGuard = ACE_Guard<ACE_Thread_Mutex>;
    ACE_Thread_Mutex m_store_mutex;
    MessageBusEndpoint m_message_bus_endpoint;
    std::unordered_map<uint64_t,AuthLink *> m_link_store;
    AuthServer *m_authserv = nullptr;
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
public:
    void        dispatch(SEGSEvent *ev) override;
                AuthHandler(AuthServer *our_server);
};

