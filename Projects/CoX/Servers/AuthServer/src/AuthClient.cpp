/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include "AuthClient.h"
#include "InternalEvents.h"
/*! \class AuthClient
	\brief A skeleton class used during authentication.
	
	This is a Client model extended with functions useful to authorization process.
*/
AuthClient::AuthClient() :m_game_server(NULL)
{
}

/*! 
	\brief Will force controlling game server to check validity of connection to this client.
*/

/*
void AuthClient::forceGameServerConnectionCheck()
{
	//ACE_ASSERT(m_game_server!=NULL);
	if(m_game_server)
		m_game_server->checkClientConnection(m_id);
}
*/
/*! 
	\brief This function checks if this client is logged in.
	\return bool, true if logged in.
*/

bool AuthClient::isLoggedIn()
{
	GameServerInterface *gs=NULL;
	ServerManagerC *sm =ServerManager::instance();
    if(m_link.getState()==ClientLinkState::LOGGED_IN) // easiest way out
	{
		return true;
	}
	// let's ask all game servers, just to stay on the safe side.
	for(size_t i=0; i<sm->MapServerCount(); i++)
	{
		gs=sm->GetGameServer(i);
		ACE_ASSERT(gs!=NULL);
        if(0==gs) // something screwy happened 
            return false;
        ClientConnectionQuery * query = new ClientConnectionQuery(0,m_account_info.account_server_id());
        ClientConnectionResponse *resp = (ClientConnectionResponse *)gs->event_target()->dispatch_sync(query);
        bool still_connected = resp->last_comm!=ACE_Time_Value::max_time;
        resp->release();
        return still_connected;
	}
	return false; //
}
