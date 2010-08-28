#pragma once
#include "AuthEvents.h"

class LoginRequest : public AuthLinkEvent
{
public:
	LoginRequest() : AuthLinkEvent(evLogin),unkval1(0),unkval2(0)
	{}
	void serializeto(GrowingBuffer &) const
	{
		ACE_ASSERT(!"Not implemented");
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 packet_code;
		buf.uGet(packet_code);
		ACE_ASSERT(packet_code==0);
		buf.uGetBytes((u8 *)login, sizeof(login));
		buf.uGetBytes((u8 *)password, sizeof(password));
		buf.uGet(unkval1);
		buf.uGet(unkval2);
	}
	char login[14];
	char password[14];
	u32 unkval1;
	u16 unkval2;
};
