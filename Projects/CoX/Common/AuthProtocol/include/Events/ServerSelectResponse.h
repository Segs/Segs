#pragma once
#include "AuthEvents.h"

class ServerSelectResponse : public AuthLinkEvent
{
public:
	ServerSelectResponse():AuthLinkEvent(evServerSelectResponse),db_server_cookie(-1),m_cookie(-1),m_unk2(-1)
	{}
	void serializefrom(GrowingBuffer &buf)
	{
		ACE_ASSERT(buf.GetReadableDataSize()>=10);
		u8 op;
		buf.uGet(op);
		buf.uGet(db_server_cookie);
		buf.uGet(m_cookie);
		buf.uGet(m_unk2);
	}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)7);
		buf.uPut(db_server_cookie);
		buf.uPut(m_cookie);
		buf.uPut(m_unk2);
	}
    void init(EventProcessor *ev_src,u32 cookie,u32 dbcookie) {m_cookie=cookie; db_server_cookie=dbcookie;m_unk2=0;}
    u32 db_server_cookie;
	u32 m_cookie;
	u8 m_unk2;
};