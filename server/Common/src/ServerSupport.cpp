/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: server_support.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "server_support.h"
ServerStopper::ServerStopper(int signum) // when instantiated adds itself to current reactor
{
	ACE_Reactor::instance()->register_handler(signum, this);
}

int ServerStopper::handle_signal (int, siginfo_t *, ucontext_t *)
{
	ACE_Reactor::instance()->end_reactor_event_loop();
	return 0;
}
