/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthServer Projects/CoX/Servers/AuthServer
 * @{
 */

#include "AuthHandler.h"

#include "AuthServer/AuthServer.h"
#include "Common/AuthProtocol/AuthLink.h"
#include "Common/Messages/Auth/AuthEvents.h"
#include "AuthDatabase/AuthDBSyncEvents.h"

#include "Common/Servers/InternalEvents.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/MessageBus.h"

#include "Components/TimeEvent.h"

#include <QDebug>

using namespace SEGSEvents;

/// Monotonically incrementing session ids, starting at 1, to make 0 special.
uint64_t AuthHandler::s_last_session_id=1;

namespace
{
    AuthorizationError s_auth_error_no_db(AUTH_ACCOUNT_SYNC_FAIL);
    AuthorizationError s_auth_error_blocked_account(AUTH_ACCOUNT_BLOCKED);
    AuthorizationError s_auth_error_db_error(AUTH_DATABASE_ERROR);
    AuthorizationError s_auth_error_unknown(AUTH_UNKN_ERROR);
    AuthorizationError s_auth_error_wrong_login_pass(AUTH_WRONG_LOGINPASS);
    AuthorizationError s_auth_error_locked_account(AUTH_ACCOUNT_BLOCKED);
    AuthorizationError s_auth_error_already_online(AUTH_ALREADY_LOGGEDIN);

    const ACE_Time_Value session_reaping_interval(0,1000*1000);
    const ACE_Time_Value link_is_stale_if_disconnected_for(2,0);
} // namespace

void AuthHandler::dispatch( Event *ev )
{
    assert(ev);
    switch(ev->type())
    {
        case evConnect:
            on_connect(static_cast<Connect *>(ev));
            break;
        case evReconnectAttempt:
            qWarning() << "Unhandled reconnect packet??";
            break;
        case evLoginRequest:
            on_login(static_cast<LoginRequest *>(ev));
            break;
        case evServerListRequest:
            on_server_list_request(static_cast<ServerListRequest *>(ev));
            break;
        case evServerSelectRequest:
            on_server_selected(static_cast<ServerSelectRequest *>(ev));
            break;
        case evDbError:
            // client sends this on exit sometimes ?
            on_disconnect(static_cast<Disconnect *>(ev));
            break;
        case evDisconnect:
            on_disconnect(static_cast<Disconnect *>(ev));
            break;
            //////////////////////////////////////////////////////////////////////////
            //  Events from other servers
            //////////////////////////////////////////////////////////////////////////
        case AuthDBEventTypes::evRetrieveAccountResponse:
            on_retrieve_account_response(static_cast<RetrieveAccountResponse *>(ev));
            break;
        case evAuthDbStatusMessage:
            on_db_error(static_cast<AuthDbStatusMessage *>(ev)); break;
        case Internal_EventTypes::evExpectClientResponse:
            on_client_expected(static_cast<ExpectClientResponse *>(ev)); break;
        case Internal_EventTypes::evClientConnectedMessage:
            on_client_connected_to_other_server(static_cast<ClientConnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evClientDisconnectedMessage:
            on_client_disconnected_from_other_server(static_cast<ClientDisconnectedMessage *>(ev));
            break;
        case Internal_EventTypes::evGameServerStatusMessage:
            on_server_status_change(static_cast<GameServerStatusMessage *>(ev));
            break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}

AuthHandler::AuthHandler(AuthServer *our_server) : m_message_bus_endpoint(*this),m_authserv(our_server)
{
    assert(HandlerLocator::getAuth_Handler()==nullptr);
    HandlerLocator::setAuth_Handler(this);
    // Note we do not store the created timer's ID anywhere, this is ok as long as we don't need to manipulate the timer
    startTimer(addTimer(session_reaping_interval), &AuthHandler::reap_stale_links);
    m_message_bus_endpoint.subscribe(evGameServerStatusMessage);
}

void AuthHandler::on_connect( Connect *ev )
{
    // TODO: guard for link state update ?
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    assert(lnk!=nullptr);
    if(lnk->get_link_stage()!=AuthLink::INITIAL)
    {
        ACE_ERROR((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),  ACE_TEXT ("Multiple connection attempts from the same addr/port")));
    }
    lnk->set_link_stage(AuthLink::CONNECTED);
    uint32_t seed = 0x1; //TODO: rand()
    lnk->init_crypto(30206,seed);
    //qWarning("Crypto seed %08x", seed);

    lnk->putq(new AuthProtocolVersion(30206,seed));
}

void AuthHandler::on_disconnect(Disconnect *ev)
{
    // since we cannot trust existence of the DisconnectEvent source at this point, we use the stored token
    if(ev->m_session_token==0)
    {
        // disconnect without correct login/session creation ?
        return;
    }
    AuthSession &session(m_sessions.session_from_token(ev->m_session_token));
    session.m_state = AuthSession::NOT_LOGGED_IN;
    if(!session.m_auth_data)
    {
        qWarning("Client disconnected without a valid login attempt. Old client ?");
    }
    {
        SessionStore::MTGuard guard(m_sessions.reap_lock());
        if(session.is_connected_to_game_server_id == 0)
            m_sessions.mark_session_for_reaping(&session, session.link()->session_token(),"AuthHander: Disconnect");
    }

    if(session.link())
    {
        session.link(nullptr);
        m_sessions.remove_from_active_sessions(&session);
    }
    // TODO: timed session reaping
}

void AuthHandler::auth_error(EventSrc *lnk,uint32_t code)
{
    lnk->putq(new AuthorizationError(code));
}

bool AuthHandler::isClientConnectedAnywhere(uint32_t client_id)
{
    uint64_t token = m_sessions.token_for_id(client_id);
    if(token==0)
        return false;
    const AuthSession &session(m_sessions.session_from_token(token));
    if(session.m_state == AuthSession::LOGGED_IN) // easiest way out
    {
        return true;
    }
    return session.is_connected_to_game_server_id!=0;
}
///
/// \fn AuthHandler::on_retrieve_account_response
/// \brief This function handles database server informing us about account details
///

void AuthHandler::on_retrieve_account_response(RetrieveAccountResponse *msg)
{
    uint64_t sess_token = msg->session_token();
    m_sessions.reap_lock().lock(); // prevent temp session from being reaped while we work with it
    if(!m_sessions.has_session_for(sess_token))
    {
        m_sessions.reap_lock().unlock();
        qWarning() << "Database response arrived to late for sess"<<sess_token;
        // we can't do anything else.
        return;
    }
    AuthSession *sess_ptr = &m_sessions.session_from_token(sess_token);
    // protector takes ownership of the session, removing it from ready-for-reaping set
    // If the protected session is not `protectee_moved` on destruction of `protector` the session is re-added to
    // the ready-for-reaping set.
    ReaperProtection<AuthSession> protector(sess_ptr,sess_token,m_sessions);
    m_sessions.reap_lock().unlock();

    AuthLink *lnk = sess_ptr->link();
    assert(lnk!=nullptr);
    if(!msg->m_data.valid()) // no account exists, return proper response
    {
        lnk->putq(s_auth_error_wrong_login_pass.shallow_copy());
        return;
    }

    RetrieveAccountResponseData & acc_inf(msg->m_data);  // all the account info you can eat!

    // pre-process the client, check if the account isn't blocked, or if the account isn't already logged in
    if(acc_inf.isBlocked())
    {
        lnk->putq(s_auth_error_locked_account.shallow_copy());
        return;
    }
    qDebug("Server Account Id : %" PRIu32, acc_inf.m_acc_server_acc_id);
    // step 3d: checking if this account is blocked
    if(isClientConnectedAnywhere(acc_inf.m_acc_server_acc_id))
    {
        lnk->putq(s_auth_error_already_online.shallow_copy());
        return;
    }
    {
        SessionStore::MTGuard guard(m_sessions.reap_lock());
        uint64_t existing_sess_tok = m_sessions.token_for_id(acc_inf.m_acc_server_acc_id);
        if(existing_sess_tok!=0)
        {
            // at this point we have two sessions, the temporary one, and the one in storage
            // the ReaperProtection above will expire on destruction, so we can simply replace the pointers
            AuthSession *old_sess_ptr = &m_sessions.session_from_token(existing_sess_tok);
            sess_token = existing_sess_tok;
            if(old_sess_ptr->m_state != AuthSession::NOT_LOGGED_IN)
                {
                    qDebug() << "Login failed";
                    lnk->putq(s_auth_error_wrong_login_pass.shallow_copy());
                    return;
                }

            // check if this session perhaps is in 'ready for reaping set', and remove it from there
            m_sessions.unmark_session_for_reaping(old_sess_ptr);
            // reset the link in the temporary session to prevent it getting closed during reaping
            sess_ptr->link(nullptr);
            sess_ptr = old_sess_ptr;
        }
        else
        {
            // a valid session and no duplicates, prevent ReaperProtection from re-adding session to the reaper set
            protector.protectee_moved();
        }
        lnk->session_token(sess_token); // record the session token in the link
        sess_ptr->link(lnk); // set the link in the session, will prevent reaping of it if it's a reused one
    }
    // create the auth data store, and move retrieved response there
    sess_ptr->m_auth_data.reset(new RetrieveAccountResponseData {acc_inf});
    m_sessions.add_to_active_sessions(sess_ptr);
    sess_ptr->m_auth_id = acc_inf.m_acc_server_acc_id;

    // if there were no errors and the provided password is valid and admin server has logged us in.
    // inform the client of the successful login attempt
    qDebug() << "Login successful";
    sess_ptr->m_state = AuthSession::LOGGED_IN;
    lnk->set_link_stage(AuthLink::AUTHORIZED);
    m_sessions.setTokenForId(sess_ptr->m_auth_id,lnk->session_token());
    lnk->putq(new LoginResponse());
}

void AuthHandler::on_login( LoginRequest *ev )
{
    AuthLink *lnk = static_cast<AuthLink *>(ev->src());
    EventProcessor *auth_db_handler=HandlerLocator::getAuthDB_Handler();
    assert(m_authserv); // if this fails it means we were not created.. ( AuthServer is creation point for the Handler)

    // if username is too long (same size as with the related char array)
    // will have no null-termination and take in data from pwd array as well
    if(ev->m_data.login[sizeof(ev->m_data.login) -1] != '\0')
    {
        lnk->putq(s_auth_error_blocked_account.shallow_copy());
        return;
    }

    // if password is too long
    if(ev->m_data.password[sizeof(ev->m_data.password)-1] != '\0')
    {
        lnk->putq(s_auth_error_blocked_account.shallow_copy());
        return;
    }

    if(!auth_db_handler)
    {
        lnk->putq(s_auth_error_no_db.shallow_copy()); // we cannot do much without that
        return;
    }

    if(lnk->get_link_stage()!=AuthLink::CONNECTED)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }
    qDebug() << "User" << ev->m_data.login.data() << "trying to login from" << lnk->peer_addr().get_host_addr();
    if(strlen(ev->m_data.login.data())<=2)
    {
        lnk->putq(s_auth_error_blocked_account.shallow_copy());
        return;
    }

    uint64_t sess_tok;
    AuthSession *session_ptr;
    // we create a temporary session here.
    {
        SessionStore::MTGuard guard(m_sessions.store_lock());
        sess_tok = s_last_session_id++;
        AuthSession &session(m_sessions.create_session(sess_tok));
        session_ptr = &session;
        session.link(lnk);
        session.set_temporary(true);
    }

    RetrieveAccountRequest *request_event =
        new RetrieveAccountRequest({ev->m_data.login.data(), ev->m_data.password.data(), 0}, sess_tok);
    request_event->src(this);
    auth_db_handler->putq(request_event);
    // here we will wait for db response, so here we're going to put the session on the read-to-reap list
    // in case db does not respond in sane time frame, the session is going to be removed.
    m_sessions.locked_mark_session_for_reaping(session_ptr,sess_tok,"AuthHandler: waiting for handover");
}

void AuthHandler::on_server_list_request( ServerListRequest *ev )
{
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->get_link_stage()!=AuthLink::AUTHORIZED)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }
    qDebug() << "Client requesting server list...";
    lnk->set_link_stage(AuthLink::CLIENT_SERVSELECT);
    ServerListResponse *r=new ServerListResponse;
    std::deque<GameServerInfo> info;
    std::vector<GameServerStatusData> status_copy;
    {
        // copy game server data to local var
        MTGuard guard(m_server_mutex);
        status_copy.reserve(m_known_game_servers.size());
        for(const std::pair<const uint8_t,GameServerStatusData> &server : m_known_game_servers)
        {
            status_copy.push_back(server.second);
        }
    }
    for(const GameServerStatusData &server : status_copy)
    {
        GameServerInfo inf;
        inf.id   = server.m_id;
        inf.addr = server.m_addr.get_ip_address();
        inf.port = server.m_addr.get_port_number();
        inf.current_players = server.m_current_players;
        inf.max_players = server.m_max_players;
        inf.online = server.m_online;
        info.emplace_back(inf);
    }
    r->set_server_list(info);
    lnk->putq(r);
}

void AuthHandler::on_server_selected(ServerSelectRequest *ev)
{
    AuthSession &session(m_sessions.session_from_event(ev));
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->get_link_stage()!=AuthLink::CLIENT_SERVSELECT)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }
    ACE_ERROR ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected server %d!\n"),ev->m_server_id));
    EventProcessor *tgt = HandlerLocator::getGame_Handler(ev->m_server_id);
    if(!tgt)
    {
        ACE_ERROR ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected non existant server !\n")));
        auth_error(lnk,rand()%33);
        return;
    }
    auto &acc_inf(*session.m_auth_data); //acc_inf.m_access_level,
    ExpectClientRequest *cl_ev = new ExpectClientRequest(
        {lnk->peer_addr(), acc_inf.m_acc_server_acc_id,acc_inf.m_access_level}, lnk->session_token());
    tgt->putq(cl_ev); // sending request to game server
    // client's state will not change until we get response from GameServer
}

void AuthHandler::on_client_expected(ExpectClientResponse *ev)
{
    AuthSession &session(m_sessions.session_from_event(ev));
    AuthLink *lnk = session.link();
    if(lnk ==nullptr)
    {
        assert(!"client disconnected before receiving game cookie");
        return;
    }
    lnk->set_link_stage(AuthLink::CLIENT_AWAITING_DISCONNECT);
    lnk->putq(new ServerSelectResponse(this,0xCAFEF00D,ev->m_data.cookie));
}

void AuthHandler::on_client_connected_to_other_server(ClientConnectedMessage *ev)
{
    assert(ev->m_data.m_server_id);
    AuthSession &session(m_sessions.session_from_token(ev->m_data.m_session));
    {
        SessionStore::MTGuard guard(m_sessions.reap_lock());
        m_sessions.unmark_session_for_reaping(&session);
    }
    session.is_connected_to_game_server_id = ev->m_data.m_server_id;
}

void AuthHandler::on_client_disconnected_from_other_server(ClientDisconnectedMessage *ev)
{
    AuthSession &session(m_sessions.session_from_token(ev->m_data.m_session));
    session.is_connected_to_game_server_id = 0;
    {
        SessionStore::MTGuard guard(m_sessions.reap_lock());
        m_sessions.mark_session_for_reaping(&session,ev->session_token(),"AuthHandler: GameServer disconnect");
    }
}

void AuthHandler::reap_stale_links()
{
    SessionStore::MTGuard guard(m_sessions.reap_lock());
    m_sessions.reap_stale_links("AuthHandler",link_is_stale_if_disconnected_for);
}

void AuthHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void AuthHandler::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

void AuthHandler::on_server_status_change(GameServerStatusMessage *ev)
{
    MTGuard guard(m_server_mutex);
    m_known_game_servers[ev->m_data.m_id] = ev->m_data;
}

void AuthHandler::on_db_error(AuthDbStatusMessage *ev)
{
    AuthSession &session(m_sessions.session_from_event(ev));
    session.link()->putq(s_auth_error_db_error.shallow_copy());
}

//! @}
