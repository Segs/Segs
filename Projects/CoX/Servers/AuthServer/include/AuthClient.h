/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef AUTHCLIENT_H
#define AUTHCLIENT_H

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "Client.h"
class GameServerInterface;

// skeleton class used during authentication
class AuthClient : public Client
{
public:
	AuthClient();
	bool			AccountBlocked() {return m_access_level==0; }
	tm				getCreationDate() const		{return m_creation_date;}
	void			setCreationDate(tm &new_date) {m_creation_date=new_date;} 
	u8 *			getPassword() const			{return (u8 *)m_password;}
	void			setPassword(const u8 *src)	{memcpy(m_password,src,14);}
	void			setState(eClientState state ) {m_state=state;};
	eClientState	getState(void) const {return m_state;};
	bool			isLoggedIn();
	void			forceGameServerConnectionCheck();
protected:
	GameServerInterface *m_game_server; //!< every auth client knows where it is connected.
	u8 m_password[14];					//!< client's password
	eClientState m_state;				//!< current state of this client
	tm m_creation_date;					//!< account creation date.

};

#endif // AUTHCLIENT_H
