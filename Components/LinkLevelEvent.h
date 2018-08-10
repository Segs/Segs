/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cassert>
#include <ace/INET_Addr.h>
#include "SEGSEvent.h"

namespace SEGSEvents {
// [[ev_def:type]]
class ConnectEvent : public Event
{
public:
    ConnectEvent(EventProcessor *ev_src) : Event(SEGS_EventTypes::evConnect,ev_src)
    {
        assert(ev_src);
    }
    ConnectEvent(EventProcessor *ev_src,const ACE_INET_Addr &addr) :
        Event(SEGS_EventTypes::evConnect,ev_src),
        src_addr(addr)
    {
        assert(ev_src);
    }
    // [[ev_def:field]]
    ACE_INET_Addr src_addr;
};
// [[ev_def:type]]
class DisconnectEvent : public Event
{
public:
    // [[ev_def:field]]
    uint64_t m_session_token;
    DisconnectEvent(EventProcessor *ev_src=nullptr) : Event(SEGS_EventTypes::evDisconnect,ev_src)
    {
    }
    DisconnectEvent(uint64_t token) : Event(SEGS_EventTypes::evDisconnect,nullptr),m_session_token(token)
    {
    }
};
} // end of SEGSEvents namespace
