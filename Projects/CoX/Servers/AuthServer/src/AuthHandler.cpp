#include "AuthHandler.h"
#include "AuthLink.h"
#include "AuthEvents.h"
#include "AuthClient.h"
#include "AdminServerInterface.h"
#include "ServerManager.h"

void AuthHandler::dispatch( SEGSEvent *ev )
{
    ACE_ASSERT(ev);
	switch(ev->type())
	{
	case SEGSEvent::evAuthConnect:
		on_connect(static_cast<ConnectEvent *>(ev)); break;
	case SEGSEvent::evLogin:
		on_login(static_cast<LoginRequest *>(ev)); break;
    case SEGSEvent::evServerListRequest:
        on_server_list_request(static_cast<ServerListRequest *>(ev)); break;
    case SEGSEvent::evServerSelectRequest:
        on_server_selected(static_cast<ServerSelectRequest *>(ev)); break;
    case SEGSEvent::evAuthDisconnect:
        on_disconnect(static_cast<DisconnectEvent *>(ev)); break;
	default:
		ACE_ASSERT(!"Unknown event encountered in dispatch.");
	}
}

void AuthHandler::on_connect( ConnectEvent *ev )
{
	// TODO: guard for link state update ?
	AuthLink *lnk=static_cast<AuthLink *>(ev->src());
	ACE_ASSERT(lnk!=0);
	if(lnk->m_state!=AuthLink::INITIAL)
	{
		ACE_ERROR((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),	ACE_TEXT ("Multiple connection attempts from the same addr/port")));
	}
	lnk->m_state=AuthLink::CONNECTED;
    u32 seed = rand();
    lnk->init_crypto(30206,seed);
	lnk->putq(new AuthorizationProtocolVersion(this,30206,seed));
}
void AuthHandler::on_disconnect( DisconnectEvent *ev )
{
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    AdminServerInterface *adminserv;
    adminserv = ServerManager::instance()->GetAdminServer();
    if(lnk->client())
    {
        lnk->client()->setState(AuthClient::NOT_LOGGEDIN);
        adminserv->Logout(lnk->client());
        //if(lnk->m_state!=AuthLink::CLIENT_AWAITING_DISCONNECT)
    }
    else
    {
        ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Client disconnected without a valid login attempt. Old client ?\n")));
    }
}
void AuthHandler::no_admin_server(EventProcessor *lnk)
{
    AuthorizationError*ev=new AuthorizationError;
    ev->init(this,4);
    lnk->putq(ev);
}
void AuthHandler::unknown_error(EventProcessor *lnk)
{
    AuthorizationError*ev=new AuthorizationError;
    ev->init(this,AUTH_UNKN_ERROR);
    lnk->putq(ev);
}
void AuthHandler::auth_error(EventProcessor *lnk,u32 code)
{
    AuthorizationError*ev=new AuthorizationError;
    ev->init(this,code);
    lnk->putq(ev);
}
void AuthHandler::on_login( LoginRequest *ev )
{
	AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    AdminServerInterface *adminserv;
    AuthClient *client = NULL;
    adminserv = ServerManager::instance()->GetAdminServer();
    ACE_ASSERT(adminserv);
    if(!adminserv)
    {
        // we cannot do much without that
        no_admin_server(lnk);
        return;
    }
    if(lnk->m_state!=AuthLink::CONNECTED)
    {
        unknown_error(lnk);
        return;
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) User %s trying to login from %s.\n"),ev->login,lnk->peer_addr().get_host_addr()));
    if(strlen(ev->login)<=2)
        return auth_error(lnk,AUTH_ACCOUNT_BLOCKED); // invalid account

    client = ServerManager::instance()->GetAuthServer()->GetClientByLogin(ev->login);
    if(!client)
    { // step 3c: creating a new account
        adminserv->SaveAccount(ev->login,ev->password); // Autocreate/save account to DB
        client = ServerManager::instance()->GetAuthServer()->GetClientByLogin(ev->login);
    }
    ACE_ASSERT(client);
    lnk->client(client); // now link knows what client it's responsible for
    eAuthError err = AUTH_WRONG_LOGINPASS; // this is default for case we don't have that client
    if(client)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\tid : %I64u\n"),client->getId()));
        // step 3d: checking if this account is blocked
        if(client->AccountBlocked())
            err = AUTH_ACCOUNT_BLOCKED;
        else if(client->isLoggedIn())
        {
            // step 3e: asking game server connection check
            client->forceGameServerConnectionCheck();
            err = AUTH_ALREADY_LOGGEDIN;
        }
        else if(client->getState()==AuthClient::NOT_LOGGEDIN)
            err = AUTH_OK;
    }
    // if there were no errors and the provided password is valid and admin server has logged us in.
    if(
        (err==AUTH_OK) &&
        (adminserv->ValidPassword(client,ev->password)) &&
        (adminserv->Login(client,lnk->peer_addr())) // this might fail somehow
        )
    {
        // inform the client of the successful login attempt 
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : succeeded\n")));
        client->setState(AuthClient::LOGGED_IN);
        lnk->m_state = AuthLink::AUTHORIZED;
        lnk->putq(new LoginResponse());
        return;
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : failed\n")));
    return auth_error(lnk,err);
}
void AuthHandler::on_server_list_request( ServerListRequest *ev )
{
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->m_state!=AuthLink::AUTHORIZED)
    {
        unknown_error(lnk);
        return;
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client requesting server list\n")));
    lnk->m_state = AuthLink::CLIENT_SERVSELECT;
    ServerListResponse *r=new ServerListResponse;
    r->set_server_list(ServerManager::instance()->getGameServerList());
    lnk->putq(r);
}
void AuthHandler::on_server_selected(ServerSelectRequest *ev)
{
    AuthLink *lnk=static_cast<AuthLink *>(ev->src());
    if(lnk->m_state!=AuthLink::CLIENT_SERVSELECT)
    {
        unknown_error(lnk);
        return;
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected server %d!\n"),ev->m_server_id));
    GameServerInterface *gs = ServerManager::instance()->GetGameServer(ev->m_server_id-1);
    if(!gs)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected non existant server !\n")));
        auth_error(lnk,rand()%33);
        return;
    }
    ServerSelectResponse *res_evt=new ServerSelectResponse;
    u32 res_cookie = gs->ExpectClient(lnk->peer_addr(),lnk->client()->getId(),lnk->client()->getAccessLevel());
    lnk->client()->setSelectedServer(gs);
    res_evt->init(this,0xCAFEF00D,res_cookie);
    lnk->m_state = AuthLink::CLIENT_AWAITING_DISCONNECT;
    lnk->putq(res_evt);
}