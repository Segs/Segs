/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientConnection.h 316 2007-01-25 15:17:16Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include "AuthConnection.h"

/******************************************************************************/
/* Servicer  --Bytes-->   ClientConnection  --Bytes-->  Protocol --Actions--> */
/* Servicer <--Messages-- ClientConnection <--Bytes--   Protocol <--Actions--    */
/******************************************************************************/
class AuthClient;

class AuthConnection_ServerSide : public AuthConnection
{
public:
	AuthConnection_ServerSide():AuthConnection(),m_client(0),fsm_data(0)
	{}
	virtual ~AuthConnection_ServerSide()
	{
		m_client = NULL;
	}
	//////////////////////////////////////////////////////////////////////////
	// Client connection FSM support methods
	//////////////////////////////////////////////////////////////////////////

	void			setClient(AuthClient *cl){m_client=cl;}
	AuthClient *	getClient(){return m_client;}

	int getClientState(void) const{return fsm_data;}; 
	void setClientState(int state) {fsm_data = state;}
protected:
	AuthClient	*m_client;
	int fsm_data;
};
#endif // CLIENTCONNECTION_H
