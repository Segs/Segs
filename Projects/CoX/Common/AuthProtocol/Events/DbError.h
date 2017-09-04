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
