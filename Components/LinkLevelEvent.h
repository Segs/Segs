/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cassert>
#include <ace/INET_Addr.h>
#include "Components/SEGSEvent.h"

namespace SEGSEvents {
// [[ev_def:type]]
class Connect : public Event
{
public:
    Connect(EventSrc *ev_src=nullptr) : Event(evConnect,ev_src)
    {
    }
    Connect(EventSrc *ev_src,const ACE_INET_Addr &addr) :
        Event(evConnect,ev_src),
        src_addr(addr)
    {
        assert(ev_src);
    }
    EVENT_IMPL(Connect)
    // [[ev_def:field]]
    ACE_INET_Addr src_addr;
};
// [[ev_def:type]]
class Disconnect : public Event
{
public:
    // [[ev_def:field]]
    uint64_t m_session_token;
    Disconnect(EventSrc *ev_src=nullptr) : Event(evDisconnect,ev_src)
    {
    }
    Disconnect(uint64_t token) : Event(evDisconnect,nullptr),m_session_token(token)
    {
    }
    EVENT_IMPL(Disconnect)
};
} // end of SEGSEvents namespace
