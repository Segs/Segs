/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ServerEndpoint.h 319 2007-01-26 17:03:18Z nemerle $
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
#ifndef WIN32
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
class ACE_INET_Addr_Hash
{
public:
	ACE_INET_Addr_Hash( ){};
	//hash_compare( Traits pred );
	size_t operator( )( const ACE_INET_Addr& Key ) const
	{
		return Key.get_ip_address()^(Key.get_port_number()<<8);
	};
};
#else
#include <hash_map>
#include <hash_set>
using namespace stdext;
class ACE_INET_Addr_Hash : public hash_compare<ACE_INET_Addr>
{
public:
	ACE_INET_Addr_Hash( ){};
	//hash_compare( Traits pred );
	size_t operator( )( const ACE_INET_Addr& Key ) const
	{
		return Key.get_ip_address()^(Key.get_port_number()<<8);
	};
	bool operator()( const ACE_INET_Addr& _Key1, const ACE_INET_Addr& _Key2 ) const
	{
		return _Key1 < _Key2;
	}
};
#endif
typedef hash_map<ACE_INET_Addr,CrudP_Protocol *,ACE_INET_Addr_Hash> hmAddrProto;
//#include "MapServer.h"
//class Net;
class LinkCommandHandler;
class PacketFactory;
class ServerEndpoint : public ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH>
{
	typedef ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH> super;
public:
	ServerEndpoint(const ACE_INET_Addr &local_addr);
	// main processing interface, all this class needs to process client requests
	virtual LinkCommandHandler *createNewHandler() const=0;
	virtual PacketFactory *getFactory() const=0;
private:
	// Part of the low level ace interface, not passed on to derived classes
	ACE_HANDLE get_handle(void) const;
	int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE); //! Called when input is available from the client.	
	int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE); //! Called when output is possible.
	int handle_timeout (const ACE_Time_Value & tv,const void *arg = 0);
	int handle_close (ACE_HANDLE handle,ACE_Reactor_Mask close_mask);//! Called when this handler is removed from the ACE_Reactor.
	int open(void *p=NULL);
protected:
	CrudP_Protocol *getClientLink(const ACE_INET_Addr &from_addr);

	list<Net *> unassociated_netlinks; // this list is cleared 
	hmAddrProto client_links;
	hash_map<u32,ACE_Time_Value> last_client_activity;

	ACE_Reactor_Notification_Strategy m_notifier;

	ACE_SOCK_Dgram endpoint_;
	// Wrapper for sending/receiving dgrams.
};
