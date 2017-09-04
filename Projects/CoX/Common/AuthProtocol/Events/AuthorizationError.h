#pragma once
#include "AuthProtocol/AuthEvents.h"

class AuthorizationError : public AuthLinkEvent
{
    uint32_t m_error_type;
public:
    AuthorizationError() : AuthLinkEvent(evAuthorizationError),m_error_type(0)
    {}
    AuthorizationError(EventProcessor *evsrc,uint32_t err) : AuthLinkEvent(evAuthorizationError,evsrc),m_error_type(err)
    {}
    void init(EventProcessor *ev_src,uint32_t error_type) {m_error_type=error_type; m_event_source=ev_src;}
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut((uint8_t)1);
        buf.uPut(m_error_type);
    }
    void serializefrom(GrowingBuffer &buf)
    {
        uint8_t op;
        buf.uGet(op);
        assert(op==1);
        buf.uGet(m_error_type);
    }
};
