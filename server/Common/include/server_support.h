/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include <ace/Reactor.h>
#include <ace/OS.h>
#include <ace/Event_Handler.h>

// this event stops main processing loop of the whole server
class ServerStopper : public ACE_Event_Handler 
{
public:
	ServerStopper(int signum = SIGINT);

	// Called when object is signaled by OS.
	virtual int handle_signal (int signum,siginfo_t * = 0,ucontext_t * = 0);
};
