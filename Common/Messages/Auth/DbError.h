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
class DbError : public AuthLinkEvent
{
public:
    // [[ev_def:field]]
    uint8_t m_err_arr[8];
    DbError() : AuthLinkEvent(evDbError)
    {}
    void init(EventSrc *ev_src,const uint8_t *error_arr) {memcpy(m_err_arr,error_arr,8);m_event_source=ev_src;}
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)3);
        buf.uPutBytes(m_err_arr,8);
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t op;
        buf.uGet(op);
        assert(op==3);
        buf.uGetBytes(m_err_arr,8);
    }
    EVENT_IMPL(DbError)
};
} // end of namespace SEGSEvents
