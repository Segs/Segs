#pragma once
#include "AuthEvents.h"

class ServerSelectResponse : public AuthLinkEvent
{
public:
    ServerSelectResponse():AuthLinkEvent(evServerSelectResponse),db_server_cookie(-1),m_cookie(-1),m_unk2(-1)
    {}
    ServerSelectResponse(EventProcessor *ev_src,uint32_t cookie,uint32_t dbcookie) : AuthLinkEvent(evServerSelectResponse,ev_src),
        db_server_cookie(dbcookie),
        m_cookie(cookie),
        m_unk2(0)
    {}
    void serializefrom(GrowingBuffer &buf)
    {
        assert(buf.GetReadableDataSize()>=10);
        uint8_t op;
        buf.uGet(op);
        buf.uGet(db_server_cookie);
        buf.uGet(m_cookie);
        buf.uGet(m_unk2);
    }
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut((uint8_t)7);
        buf.uPut(db_server_cookie);
        buf.uPut(m_cookie);
        buf.uPut(m_unk2);
    }
    void init(EventProcessor *ev_src,uint32_t cookie,uint32_t dbcookie) {m_cookie=cookie; db_server_cookie=dbcookie;m_unk2=0; m_event_source=ev_src;}
    uint32_t db_server_cookie;
    uint32_t m_cookie;
    uint8_t m_unk2;
};
