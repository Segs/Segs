/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: NetManager.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include "GameProtocol.h"
#include "GameProtocolHandler.h"
/*
	This class handles creation, selection, and destruction of Net class instances
	also, it handles first Client message ( which tells us protocol version )
*/
class GameProtocolManager
{
public:
	static IGameProtocol *CreateGameProtocol();
//	IGameProtocol *getProtocolFor(ACE_INET_Addr &clientaddr); // if client has
};
