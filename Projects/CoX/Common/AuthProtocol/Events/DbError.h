/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "AuthProtocol/AuthEvents.h"

class DbError : public AuthLinkEvent
{
    uint8_t m_err_arr[8];
public:
    DbError() : AuthLinkEvent(evDbError)
    {}
    void init(EventProcessor *ev_src,const uint8_t *error_arr) {memcpy(m_err_arr,error_arr,8);m_event_source=ev_src;}
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut((uint8_t)3);
        buf.uPutBytes(m_err_arr,8);
    }
    void serializefrom(GrowingBuffer &buf)
    {
        uint8_t op;
        buf.uGet(op);
        assert(op==3);
        buf.uGetBytes(m_err_arr,8);
    }
};
