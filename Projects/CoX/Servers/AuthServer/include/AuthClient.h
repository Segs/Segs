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
class AuthClient : public ClientSession
{

public:
	                AuthClient();
	bool			isLoggedIn();
	void			forceGameServerConnectionCheck();
	void			setSelectedServer(IGameServer *gs){m_game_server=gs;};
protected:
	IGameServer *   m_game_server; //!< every auth client knows where it is connected.

};

#endif // AUTHCLIENT_H
