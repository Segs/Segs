/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServerEndpoint.h 292 2006-10-12 14:12:14Z nemerle $
 */

#pragma once
#include <string>
#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>

#include "Base.h"
#include "CRUDP_Protocol.h"
#include "ServerEndpoint.h"
#include "ClientHandler.h"
// Endpoint is a ACE's handler for incoming packets and such
class GameServerEndpoint : public ServerEndpoint
{
//	typedef ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH> super;
	GameServer *m_server; //! this is used to inform all subordinate character handlers who's the boss
public:
	GameServerEndpoint(const ACE_INET_Addr &addr,GameServer *srv):ServerEndpoint(addr),m_server(srv){}
	virtual LinkCommandHandler *createNewHandler() const 
	{
		return new CharacterHandler(m_server);
	};
	virtual PacketFactory *getFactory() const;
};
