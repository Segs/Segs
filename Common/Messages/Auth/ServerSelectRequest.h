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
class ServerSelectRequest : public AuthLinkEvent
{
public:
    ServerSelectRequest() : AuthLinkEvent(evServerSelectRequest)
    {}
    void init(EventSrc *ev_src,uint8_t server_id)
    {
        m_server_id    = server_id;
        m_event_source = ev_src;
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t op;
        buf.uGet(op);
        buf.uGetBytes(unkLoginArray, sizeof(unkLoginArray));
        buf.uGet(m_server_id);
    }
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)2);
        buf.uPutBytes(unkLoginArray, sizeof(unkLoginArray));
        buf.uPut(m_server_id);
    }
    // [[ev_def:field]]
    uint8_t unkLoginArray[8];
    // [[ev_def:field]]
    uint8_t m_server_id = 0;
    EVENT_IMPL(ServerSelectRequest)
};
} //end of namespace SEGSEvents

