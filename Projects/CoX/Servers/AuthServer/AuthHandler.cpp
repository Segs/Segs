#include "AuthHandler.h"

#include "AuthServer/AuthServer.h"
#include "AuthProtocol/AuthLink.h"
#include "AuthProtocol/AuthEvents.h"

#include "AdminServer/AccountInfo.h"
#include "Servers/AdminServerInterface.h"
#include "Servers/ServerManager.h"
#include "Servers/InternalEvents.h"
#include "Servers/HandlerLocator.h"
#include "Servers/MessageBus.h"

#include <QDebug>

/// Monotonically incrementing session ids, starting at 1, to make 0 special.
uint64_t AuthHandler::s_last_session_id=1;
namespace {
static AuthorizationError s_auth_error_no_db(AUTH_ACCOUNT_SYNC_FAIL);
static AuthorizationError s_auth_error_blocked_account(AUTH_ACCOUNT_BLOCKED);
static AuthorizationError s_auth_error_db_error(AUTH_DATABASE_ERROR);
static AuthorizationError s_auth_error_unknown(AUTH_UNKN_ERROR);
static AuthorizationError s_auth_error_wrong_login_pass(AUTH_WRONG_LOGINPASS);
static AuthorizationError s_auth_error_locked_account(AUTH_ACCOUNT_BLOCKED);
static AuthorizationError s_auth_error_already_online(AUTH_ALREADY_LOGGEDIN);
}
void AuthHandler::dispatch( SEGSEvent *ev )
{
    assert(ev);
    switch(ev->type())
    {
        case SEGS_EventTypes::evConnect:
            on_connect(static_cast<ConnectEvent *>(ev));
            break;
        case evLogin:
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
            on_disconnect(static_cast<DisconnectEvent *>(ev));
            break;
        case SEGS_EventTypes::evDisconnect:
            on_disconnect(static_cast<DisconnectEvent *>(ev));
            break;
            //////////////////////////////////////////////////////////////////////////
            //  Events from other servers
            //////////////////////////////////////////////////////////////////////////
        case Internal_EventTypes::evExpectClientResponse:
            on_client_expected(static_cast<ExpectClientResponse *>(ev)); break;
        default:
            assert(!"Unknown event encountered in dispatch.");
    }
}
AuthHandler::AuthHandler(AuthServer *our_server) : m_message_bus_endpoint(*this),m_authserv(our_server)
{
    assert(HandlerLocator::getAuth_Handler()==nullptr);
    HandlerLocator::setAuth_Handler(this);
//    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientConnected);
//    m_message_bus_endpoint.subscribe(Internal_EventTypes::evClientDisconnected);
}
void AuthHandler::on_connect( ConnectEvent *ev )
{
    // TODO: guard for link state update ?
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    assert(lnk!=nullptr);
    if(lnk->m_state!=AuthLink::INITIAL)
    {
        ACE_ERROR((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),  ACE_TEXT ("Multiple connection attempts from the same addr/port")));
    }
    lnk->m_state=AuthLink::CONNECTED;
    uint32_t seed = 0x1; //TODO: rand()
    lnk->init_crypto(30206,seed);
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Crypto seed %08x\n"), seed ));
    {
        SessionStore::MTGuard guard(m_sessions.store_lock());
        AuthSession &session(m_sessions.createSession(s_last_session_id));
        lnk->session_token(s_last_session_id); // record the session token in the link
        session.m_link = lnk; // create session and set the link in it
        ++s_last_session_id;
    }
    lnk->putq(new AuthorizationProtocolVersion(30206,seed));
}
void AuthHandler::on_disconnect( DisconnectEvent *ev )
{
    AuthSession &session(m_sessions.sessionFromEvent(ev));
    session.m_state = AuthSession::NOT_LOGGED_IN;
    if(!session.m_auth_data)
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Client disconnected without a valid login attempt. Old client ?\n")));
    }
    if(session.m_link)
    {
        session.m_link = nullptr;
        m_sessions.removeFromActiveSessions(&session);
    }
    // TODO: timed session reaping
}
void AuthHandler::auth_error(EventProcessor *lnk,uint32_t code)
{
    lnk->putq(new AuthorizationError(code));
}
bool AuthHandler::isSessionConnectedAnywhere(uint64_t ses)
{
    GameServerInterface *gs=nullptr;
    ServerManagerC *sm =ServerManager::instance();

    const AuthSession &session(m_sessions.sessionFromToken(ses));
    if(session.m_state == AuthSession::LOGGED_IN) // easiest way out
    {
        return true;
    }
    // let's ask all game servers, just to stay on the safe side.
    ClientConnectionRequest query({session.m_auth_id},ses);
    for(size_t i=0; i<sm->MapServerCount(); i++)
    {
        gs=sm->GetGameServer(i);
        assert(gs!=nullptr);
        if(nullptr==gs) // something screwy happened
            return false;
        ClientConnectionResponse *resp = (ClientConnectionResponse *)gs->event_target()->dispatchSync(query.shallow_copy());
        assert(resp->session_token()==ses);
        const ClientConnectionResponseData &resp_data(resp->m_data);
        bool still_connected = resp_data.last_comm!=ACE_Time_Value::max_time;
        resp->release();
        if(still_connected)
            return still_connected;
    }
    return false;
}
void AuthHandler::on_login( LoginRequest *ev )
{
    AdminServerInterface *adminserv = ServerManager::instance()->GetAdminServer();
    AuthSession &session(m_sessions.sessionFromEvent(ev));
    AuthLink *lnk = static_cast<AuthLink *>(ev->src());
    assert(adminserv);
    assert(m_authserv); // if this fails it means we were not created.. ( AuthServer is creation point for the Handler)

    if(!adminserv)
    {
        // we cannot do much without that
        lnk->putq(s_auth_error_no_db.shallow_copy());
        return;
    }

    if(lnk->m_state!=AuthLink::CONNECTED)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }

    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) User %s trying to login from %s.\n"),ev->m_data.login,lnk->peer_addr().get_host_addr()));
    if(strlen(ev->m_data.login)<=2)
        return auth_error(lnk,AUTH_ACCOUNT_BLOCKED); // invalid account

    session.m_auth_data.reset(new AuthAccountData);

    m_authserv->GetClientByLogin(ev->m_data.login,*session.m_auth_data);
    // TODO: Version 0.3 will need to use admin tools instead of creating accounts willy-nilly
    if(!session.m_auth_data->valid()) // no account exists, create one ( step 3c )
    {
        adminserv->SaveAccount(ev->m_data.login,ev->m_data.password); // Autocreate/save account to DB
        m_authserv->GetClientByLogin(ev->m_data.login,*session.m_auth_data);
    }
    if(!session.m_auth_data->valid()) {
        ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) User %s from %s - couldn't get/create account.\n"),ev->m_data.login,lnk->peer_addr().get_host_addr()));
        lnk->putq(s_auth_error_db_error.shallow_copy());
        return;
    }

    m_sessions.addToActiveSessions(&session);
    AuthAccountData & acc_inf(*session.m_auth_data);  // all the account info you can eat!
    session.m_auth_id = acc_inf.m_acc_server_acc_id;
    bool no_errors=false;                           // this flag is set if there were no errors during client pre-processing
    // pre-process the client, check if the account isn't blocked, or if the account isn't already logged in
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\tid : %I64u\n"),acc_inf.m_acc_server_acc_id));
    // step 3d: checking if this account is blocked
    if(acc_inf.isBlocked()) {
        lnk->putq(s_auth_error_locked_account.shallow_copy());
        return;
    }
    if(isSessionConnectedAnywhere(lnk->session_token()))
    {
        // step 3e: asking game server connection check
        // TODO: client->forceGameServerConnectionCheck();
        lnk->putq(s_auth_error_already_online.shallow_copy());
        return;
    }
    else if(session.m_state == AuthSession::NOT_LOGGED_IN)
        no_errors = true;
    // if there were no errors and the provided password is valid and admin server has logged us in.
    if(
            no_errors &&
            adminserv->ValidPassword(acc_inf,ev->m_data.password) &&
            adminserv->Login(acc_inf,lnk->peer_addr()) // this might fail somehow
            )
    {
        // inform the client of the successful login attempt
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : succeeded\n")));
        session.m_state = AuthSession::LOGGED_IN;
        lnk->m_state = AuthLink::AUTHORIZED;
        lnk->putq(new LoginResponse());
    }
    else
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : failed\n")));
        lnk->putq(s_auth_error_wrong_login_pass.shallow_copy());
    }
}
void AuthHandler::on_server_list_request( ServerListRequest *ev )
{
    AuthSession &session(m_sessions.sessionFromEvent(ev));
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->m_state!=AuthLink::AUTHORIZED)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client requesting server list\n")));
    lnk->m_state = AuthLink::CLIENT_SERVSELECT;
    ServerListResponse *r=new ServerListResponse;
    const ServerManagerC::dGameServer *servers = ServerManager::instance()->getGameServerList();
    std::deque<GameServerInfo> info;
    for(size_t idx=0; idx<servers->size(); ++idx) {
        GameServerInfo inf;
        GameServerInterface *iface = servers->at(idx);
        inf.id=iface->getId();
        inf.addr = iface->getAddress().get_ip_address();
        inf.port = iface->getAddress().get_port_number();
        inf.current_players = iface->getCurrentPlayers();
        inf.max_players = iface->getMaxPlayers();
        inf.online = true;
        info.push_back(inf);
    }
    r->set_server_list(info);
    lnk->putq(r);
}
void AuthHandler::on_server_selected(ServerSelectRequest *ev)
{
    AuthSession &session(m_sessions.sessionFromEvent(ev));
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->m_state!=AuthLink::CLIENT_SERVSELECT)
    {
        lnk->putq(s_auth_error_unknown.shallow_copy());
        return;
    }
    ACE_ERROR ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected server %d!\n"),ev->m_server_id));
    GameServerInterface *gs = ServerManager::instance()->GetGameServer(ev->m_server_id-1);
    if(!gs)
    {
        ACE_ERROR ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected non existant server !\n")));
        auth_error(lnk,rand()%33);
        return;
    }
    AuthAccountData &acc_inf(*session.m_auth_data); //acc_inf.m_access_level,
    ExpectClientRequest *cl_ev=new ExpectClientRequest({acc_inf.m_acc_server_acc_id,lnk->peer_addr()},lnk->session_token());
    gs->event_target()->putq(cl_ev); // sending request to game server
    // client's state will not change until we get response from GameServer
}
void AuthHandler::on_client_expected(ExpectClientResponse *ev)
{
    AuthSession &session(m_sessions.sessionFromEvent(ev));
    if(session.m_link==nullptr)
    {
        assert(!"client disconnected before receiving game cookie");
        return;
    }
    AuthLink *lnk = session.m_link;
    lnk->m_state = AuthLink::CLIENT_AWAITING_DISCONNECT;
    lnk->putq(new ServerSelectResponse(this,0xCAFEF00D,ev->m_data.cookie));
}
