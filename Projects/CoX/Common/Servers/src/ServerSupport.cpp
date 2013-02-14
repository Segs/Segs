/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "server_support.h"
ServerStopper::ServerStopper(int signum) // when instantiated adds itself to current reactor
{
    ACE_Reactor::instance()->register_handler(signum, this);
}

int ServerStopper::handle_signal (int, siginfo_t *s_i, ucontext_t *u_c)
{
    exit(0); //TODO: this is not graceful :(
    return 0;
}
