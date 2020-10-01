/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Auth/AuthEvents.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class AuthorizationError : public AuthLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_error_type = 0;
    AuthorizationError() : AuthLinkEvent(evAuthorizationError)
    {}
    AuthorizationError(uint32_t err) : AuthLinkEvent(evAuthorizationError),m_error_type(err)
    {}
    void init(EventSrc *ev_src,uint32_t error_type) {m_error_type=error_type; m_event_source=ev_src;}
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)1);
        buf.uPut(m_error_type);
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t op;
        buf.uGet(op);
        assert(op==1);
        buf.uGet(m_error_type);
    }
    EVENT_IMPL(AuthorizationError)
};
}
