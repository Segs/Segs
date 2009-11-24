#pragma once
#include "AuthEvents.h"

class ServerSelectRequest : public AuthLinkEvent
{
public:
	ServerSelectRequest() : AuthLinkEvent(evServerSelectRequest),m_server_id(0)
	{}
	void init(EventProcessor *ev_src,u8 server_id) {m_server_id=server_id; m_event_source=ev_src;}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 op;
		buf.uGet(op);
		buf.uGetBytes(unkLoginArray, sizeof(unkLoginArray));
		buf.uGet(m_server_id);
	}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)2);
		buf.uPutBytes(unkLoginArray, sizeof(unkLoginArray));
		buf.uPut(m_server_id);
	}
	u8 unkLoginArray[8];
	u8 m_server_id;
};
