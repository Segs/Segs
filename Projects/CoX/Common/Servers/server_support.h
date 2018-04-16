/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/Reactor.h>
#include <ace/ACE.h>
#include <ace/Event_Handler.h>
#include <ace/OS_NS_signal.h>

// this event stops main processing loop of the whole server
class ServerStopper : public ACE_Event_Handler
{
    void (*shut_down_func)();
public:
    ServerStopper(int signum = SIGINT,void (*func)()=nullptr);

    // Called when object is signaled by OS.
    virtual int handle_signal (int signum,siginfo_t * = 0,ucontext_t * = 0);
};
