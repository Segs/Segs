/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthFSM.cpp 271 2006-10-02 04:30:50Z nemerle $
 */

#include <stdlib.h>
#include "Base.h"
#include "AuthFSM.h"
#include "AuthPacket.h"
#include "AuthProtocol.h"
#include "ClientConnection.h"
#include "AuthClient.h"
#include "AdminServerInterface.h"
#include "ServerManager.h"
#include "AuthServer.h"
/*
typedef enum
{
	AUTH_OK = 0,
	AUTH_ACCOUNT_BLOCKED,
	AUTH_WRONG_LOGINPASS,
	AUTH_ALREADY_LOGGEDIN,
	AUTH_UNAVAILABLE_SERVER,
	AUTH_KICKED_FROM_GAME 
} eAuthError;
*/
AuthPacket *AuthFSM_Default::auth_error(char a,char b)
{
	AuthPacket *result = AuthPacketFactory::PacketForType(SMSG_AUTH_ERROR);
	static_cast<pktAuthErrorPacket*>(result)->setError(2,b);
	return result;
}
AuthPacket *AuthFSM_Default::ReceivedPacket(AuthConnection *conn,AuthPacket *pkt)
{
	AuthConnection_ServerSide *caller = static_cast<AuthConnection_ServerSide *>(conn);
	AuthPacket *result = NULL;
	AdminServerInterface *adminserv;
	adminserv = ServerManager::instance()->GetAdminServer();
	ACE_ASSERT(adminserv);
	if(!adminserv)
	{
		// we cannot do much without that
		AuthPacketFactory::Destroy(pkt);
		result = AuthPacketFactory::PacketForType(SMSG_AUTH_ERROR);
		static_cast<pktAuthErrorPacket*>(result)->setError(2,4);
		return result;
	}
	switch((eClientState)caller->getClientState())
	{
	case CLIENT_CONNECTED: //step 3 of ClientScenario1
		{
			AuthClient *client = NULL;
			if(pkt->GetPacketType()!=CMSG_AUTH_LOGIN)
			{
				ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unexpected packet type %d while waiting for login attempt\n"),static_cast<int>(pkt->GetPacketType())));
				result = auth_error(2,AuthServer::AUTH_UNKN_ERROR);
				break;
			}
			pktAuthLogin *auth_pkt = static_cast<pktAuthLogin *>(pkt);
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) User %s trying to login from %s.\n"),auth_pkt->username,caller->peer().get_host_addr()));
			// step 3b: retrieving client's info
			client = ServerManager::instance()->GetAuthServer()->GetClientByLogin(auth_pkt->username);
			if(!client)
			{ // step 3c: creating a new account
				adminserv->SaveAccount(auth_pkt->username,auth_pkt->password); // Autocreate/save account to DB
				client = ServerManager::instance()->GetAuthServer()->GetClientByLogin(auth_pkt->username);
			}
			ACE_ASSERT(client);
			caller->setClient(client);
			//client->
			AuthServer::eAuthError err = AuthServer::AUTH_WRONG_LOGINPASS; // this is default for case we don't have that client
			if(client)
			{
				ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\tid : %I64u\n"),client->getId()));
				// step 3d: checking if this account is blocked
				if(client->AccountBlocked())
					err = AuthServer::AUTH_ACCOUNT_BLOCKED;
				else if(client->isLoggedIn())
				{
					// step 3e: asking game server connection check
					client->forceGameServerConnectionCheck();
					err = AuthServer::AUTH_ALREADY_LOGGEDIN;
				}
				else if(client->getState()==AuthClient::NOT_LOGGEDIN)
					err = AuthServer::AUTH_OK;
			}
			if(
				(err==AuthServer::AUTH_OK) &&
				(adminserv->ValidPassword(client,auth_pkt->password)) &&
				(adminserv->Login(client,caller->peer())) // this might fail somehow
			   )
			{
				client->setState(AuthClient::LOGGED_IN);
				result = AuthPacketFactory::PacketForType(PKT_AUTH_LOGIN_SUCCESS);
				caller->setClientState((int)CLIENT_AUTHORIZED);
				caller->setClient(client);
				ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : succeeded\n")));
			}
			else
			{
				result = auth_error(2,err);
				ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : failed\n")));
			}
			break;
		}
	case CLIENT_AUTHORIZED:
		if(pkt->GetPacketType()!=CMSG_AUTH_REQUEST_SERVER_LIST)
		{
			ACE_DEBUG ((LM_WARNING,ACE_TEXT ("(%P|%t) Unexpected packet type %d while waiting for server list request\n"),static_cast<int>(pkt->GetPacketType())));
			result = auth_error(2,AuthServer::AUTH_UNKN_ERROR);
			break; // let's do something here. Maybe send error and close the connection ?
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client requesting server list\n")));
		result = AuthPacketFactory::PacketForType(SMSG_AUTH_SERVER_LIST);
		caller->setClientState((int)CLIENT_SERVSELECT);
		break;
	case CLIENT_SERVSELECT:
		if(pkt->GetPacketType()!=CMSG_AUTH_SELECT_DBSERVER)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client sent wrong packet\n")));
			result = auth_error(2,AuthServer::AUTH_UNKN_ERROR);
		}
		else
		{
			pktAuthSelectServer *serv_select = static_cast<pktAuthSelectServer *>(pkt);
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected server %d!\n"),serv_select->serverId));
			pktAuthSelectServerResponse *res_pkt=static_cast<pktAuthSelectServerResponse *>(AuthPacketFactory::PacketForType(PKT_SELECT_SERVER_RESPONSE));
			GameServerInterface *gs = ServerManager::instance()->GetGameServer(serv_select->serverId-1);
			if(!gs)
			{
				ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Client selected non existant server !\n")));
				result = AuthPacketFactory::PacketForType(SMSG_AUTH_ERROR);
				static_cast<pktAuthErrorPacket*>(result)->setError(2,rand()%33);
				break;
			}
			u32 res_cookie = gs->ExpectClient(caller->peer(),caller->getClient()->getId(),caller->getClient()->getAccessLevel());
			res_pkt->cookie=0xCAFEF00D;
			res_pkt->db_server_cookie=res_cookie;
			result = res_pkt;
			caller->setClientState((int)CLIENT_AWAITING_DISCONNECT);
		}
		break;
	case CLIENT_UNCONNECTED:
	case CLIENT_AWAITING_DISCONNECT:
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : Recv packet while in non-receiving mood\n")));
	default: //-fallthrough
		result = AuthPacketFactory::PacketForType(SMSG_AUTH_ERROR);
		static_cast<pktAuthErrorPacket*>(result)->setError(2,rand()%33);
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\t\t : failed\n")));
		break;

	}
	AuthPacketFactory::Destroy(pkt);
	return result;
}
AuthPacket *AuthFSM_Default::ConnectionEstablished(AuthConnection *conn)
{
	AuthConnection_ServerSide *caller = static_cast<AuthConnection_ServerSide *>(conn);

	// this is a step 2 from ClientScenario1
	pktAuthVersion * res = (pktAuthVersion *)AuthPacketFactory::PacketForType(SMSG_AUTHVERSION);
	caller->setClientState((int)CLIENT_CONNECTED);
	return res;
}
void AuthFSM_Default::ConnectionClosed(AuthConnection *conn)
{
	AuthConnection_ServerSide *caller = static_cast<AuthConnection_ServerSide *>(conn);
	AdminServerInterface *adminserv;
	adminserv = ServerManager::instance()->GetAdminServer();
	if(caller->getClient())
	{
		caller->getClient()->setState(AuthClient::NOT_LOGGEDIN);
		adminserv->Logout(caller->getClient());
	}
	else
	{
		ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Client disconnected without a valid login attempt. Old client ?\n")));
	}
}
