/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "server_support.h"

#include "HandlerLocator.h"
#include "MessageBus.h"

ServerStopper::ServerStopper(int signum,void (*func)()) // when instantiated adds itself to current reactor
{
    ACE_Reactor::instance()->register_handler(signum, this);
    shut_down_func = func;
}

int ServerStopper::handle_signal (int, siginfo_t */*s_i*/, ucontext_t */*u_c*/)
{
    shutDownAllActiveHandlers();
    shutDownMessageBus();
    ACE_Reactor::instance()->end_reactor_event_loop();
    if(shut_down_func)
        shut_down_func();
    return 0;
}
