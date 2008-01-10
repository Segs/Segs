/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameProtocolHandler.h 319 2007-01-26 17:03:18Z nemerle $
 */

#pragma once
#include <deque>
#include <algorithm>

#include <map>
#include <set>
#include <ace/INET_Addr.h>
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
using namespace stdext;
#endif
//#include "Packet.h"
using namespace std;
//using namespace stdext; 
class GamePacket;
class ControlPacket;
class IClient;
class IGameProtocol;

/*!
	LinkCommandHandler is a class that implements common parts of client server protocol
	i.e. commands that are sent by any part of system to any other part 
*/
class LinkCommandHandler 
{
public:
virtual					~LinkCommandHandler();
		void			setTargetAddr(const ACE_INET_Addr &ad) {m_target_location=ad;};
		ACE_INET_Addr &	getTargetAddr(){return m_target_location;}
		void			setProtocol(IGameProtocol *my_proto){m_proto=my_proto;};
virtual bool			ReceivePacket(GamePacket *pak) = 0;
virtual void			setClient(IClient *cl) = 0; //! used to link a handler with underlying client object
protected:
		bool			ReceiveControlPacket(const ControlPacket *);
		IGameProtocol *	m_proto;
		ACE_INET_Addr	m_target_location; //!< this tells us what is our target location ( where to send our packets to)
};
