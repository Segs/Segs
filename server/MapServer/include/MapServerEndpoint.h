/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapServerEndpoint.h 291 2006-10-12 10:52:55Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef MAPSERVERENDPOINT_H
#define MAPSERVERENDPOINT_H

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
#include "MapHandler.h"

class MapServerEndpoint : public ServerEndpoint
{
	MapServer *m_server; //! this is used to inform all subordinate character handlers who's the boss

public:
	MapServerEndpoint(const ACE_INET_Addr &addr,MapServer *srv) : ServerEndpoint(addr),m_server(srv){}
	virtual LinkCommandHandler *createNewHandler() const
	{
		return new MapHandler(m_server);
	};
	virtual PacketFactory *getFactory() const;

};

#endif // MAPSERVERENDPOINT_H
