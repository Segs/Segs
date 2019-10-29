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
class ServerSelectResponse : public AuthLinkEvent
{
public:
    ServerSelectResponse():AuthLinkEvent(evServerSelectResponse)
    {}
    ServerSelectResponse(EventSrc *ev_src,uint32_t cookie,uint32_t dbcookie) : AuthLinkEvent(evServerSelectResponse,ev_src),
        db_server_cookie(dbcookie),
        m_cookie(cookie),
        m_unk2(0)
    {}
    void serializefrom(GrowingBuffer &buf) override
    {
        assert(buf.GetReadableDataSize()>=10);
        uint8_t op;
        buf.uGet(op);
        buf.uGet(db_server_cookie);
        buf.uGet(m_cookie);
        buf.uGet(m_unk2);
    }
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)7);
        buf.uPut(db_server_cookie);
        buf.uPut(m_cookie);
        buf.uPut(m_unk2);
    }
    void init(EventSrc *ev_src, uint32_t cookie, uint32_t dbcookie)
    {
        m_cookie         = cookie;
        db_server_cookie = dbcookie;
        m_unk2           = 0;
        m_event_source   = ev_src;
    }
    // [[ev_def:field]]
    uint32_t db_server_cookie=~0U;
    // [[ev_def:field]]
    uint32_t m_cookie = ~0U;
    // [[ev_def:field]]
    uint8_t m_unk2 = 0xFF;
    EVENT_IMPL(ServerSelectResponse)
};
} // end of namespace SEGSEvents
