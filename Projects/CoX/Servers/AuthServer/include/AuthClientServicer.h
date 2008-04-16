/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthClientServicer.h 317 2007-01-25 15:44:29Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef AUTHCLIENTSERVICER_H
#define AUTHCLIENTSERVICER_H

#include <ace/ACE.h>
#include <ace/Synch.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Queue.h>
#include <ace/Message_Block.h>
#include <ace/Event_Handler.h>
#include <ace/Svc_Handler.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "ClientConnection.h"

class AuthClientService : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
	ACE_Reactor_Notification_Strategy m_notifier;
	AuthConnection *client; 
public:
			AuthClientService(void);
virtual		~AuthClientService(void);

		int open(void * = 0);									//!< Called when we start to service a new connection
virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);	//!< Called when input is available from the client.
virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);	//!< Called when output is possible.
virtual int handle_close(ACE_HANDLE handle,ACE_Reactor_Mask close_mask);//!< Called when this handler is removed from the ACE_Reactor.
};

#endif // AUTHCLIENTSERVICER_H
