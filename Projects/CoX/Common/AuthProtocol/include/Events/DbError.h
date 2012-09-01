#pragma once
#include "AuthEvents.h"

class DbError : public AuthLinkEvent
{
	u8 m_err_arr[8];
public:
	DbError() : AuthLinkEvent(evDbError)
	{}
	void init(EventProcessor *ev_src,const u8 *error_arr) {memcpy(m_err_arr,error_arr,8);m_event_source=ev_src;}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)3);
		buf.uPutBytes(m_err_arr,8);
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 op;
		buf.uGet(op);
		assert(op==3);
		buf.uGetBytes(m_err_arr,8);
	}
};
