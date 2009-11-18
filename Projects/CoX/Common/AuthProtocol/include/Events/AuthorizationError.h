#pragma once
#include "AuthEvents.h"

class AuthorizationError : public AuthLinkEvent
{
	u32 m_error_type;
public:
	AuthorizationError() : AuthLinkEvent(evAuthorizationError),m_error_type(0)
	{}
	void init(EventProcessor *ev_src,u32 error_type) {m_error_type=error_type; m_event_source=ev_src;}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)1);
		buf.uPut(m_error_type);
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 op;
		buf.uGet(op);
		ACE_ASSERT(op==1);
		buf.uGet(m_error_type);
	}
};
