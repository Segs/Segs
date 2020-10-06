/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Servers/InternalEvents.h"
#include "AuthDatabase/AuthDBSyncEvents.h"
#include "Common/AuthProtocol/AuthLink.h"
#include "Common/Messages/Auth/AuthEvents.h"
#include "Common/Servers/MessageBusEndpoint.h"
#include "Common/Servers/ClientManager.h"
#include "Components/EventProcessor.h"

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <map>

class AuthServer;

namespace SEGSEvents
{
struct RetrieveAccountResponse;
struct ValidatePasswordResponse;
class Timeout;
}

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

struct AuthSession : public ClientSession
{
    enum eClientState
    {
        CLIENT_DISCONNECTED = 0,
        CLIENT_EXPECTED,
        NOT_LOGGED_IN,
        LOGGED_IN,
        CLIENT_CONNECTED
    };

    uint32_t        auth_id() const { return m_auth_id; }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // those functions store 'temporariness' state of the link in the lowest bit of the pointer
    void            set_temporary(bool v) { (intptr_t &)(m_link) = (intptr_t(m_link) & ~1) | intptr_t(v); }
    bool            is_temporary() const { return intptr_t(m_link) & 1; }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    AuthLink *      link() { return (AuthLink *)(intptr_t(m_link) & ~1); }
    /// \note setting the link does not preserver the state of the previous one.
    void            link(AuthLink *l) { m_link = l; }

    std::unique_ptr<SEGSEvents::RetrieveAccountResponseData> m_auth_data;
    uint32_t        m_auth_id=0;
    eClientState    m_state = NOT_LOGGED_IN;
    uint32_t        is_connected_to_game_server_id=0;
protected:
    AuthLink *      m_link = nullptr;
};
class AuthHandler : public EventProcessor
{
    using SessionStore = ClientSessionStore<AuthSession>;
    using MTGuard = ACE_Guard<ACE_Thread_Mutex>;
    using ServerMap = std::map<uint8_t,SEGSEvents::GameServerStatusData>;
protected:
    IMPL_ID(AuthHandler)
    static uint64_t         s_last_session_id;
    MessageBusEndpoint      m_message_bus_endpoint;
    SessionStore            m_sessions;
    AuthServer              *m_authserv = nullptr;
    ACE_Thread_Mutex        m_server_mutex;
    ServerMap               m_known_game_servers;

    bool        isClientConnectedAnywhere(uint32_t client_id);
    void        reap_stale_links();
    void        serialize_from(std::istream &is) override;
    void        serialize_to(std::ostream &is) override;

    //////////////////////////////////////////////////////////////////////////
    // Message bus subscriptions
    void        on_server_status_change(SEGSEvents::GameServerStatusMessage *ev);
    //////////////////////////////////////////////////////////////////////////
    // function that send messages into the link
    void        auth_error(EventSrc *lnk, uint32_t code);
    //////////////////////////////////////////////////////////////////////////
    // incoming link level event handlers
    void        on_connect(SEGSEvents::Connect *ev);
    void        on_disconnect(SEGSEvents::Disconnect *ev);
    void        on_login( SEGSEvents::LoginRequest *ev );
    void        on_server_list_request( SEGSEvents::ServerListRequest *ev );
    void        on_server_selected(SEGSEvents::ServerSelectRequest *ev);

    //////////////////////////////////////////////////////////////////////////
    // Server <-> server event handlers
    void        on_retrieve_account_response(SEGSEvents::RetrieveAccountResponse *msg);
    void        on_client_expected(SEGSEvents::ExpectClientResponse *ev);
    void        on_client_connected_to_other_server(SEGSEvents::ClientConnectedMessage *ev);
    void        on_client_disconnected_from_other_server(SEGSEvents::ClientDisconnectedMessage *ev);
    void        on_db_error(SEGSEvents::AuthDbStatusMessage *ev);

    void        dispatch(SEGSEvents::Event *ev) override;
public:
                AuthHandler(AuthServer *our_server);
};
