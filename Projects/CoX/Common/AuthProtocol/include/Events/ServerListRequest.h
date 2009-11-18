#pragma once
#include "AuthEvents.h"

class ServerListRequest : public AuthLinkEvent
{
public:
	ServerListRequest() : AuthLinkEvent(evServerListRequest),unk1(-1)
	{}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)5);
		buf.uPutBytes(unkArrayFromLoginResponse, sizeof(unkArrayFromLoginResponse));
		buf.uPut(unk1);	
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 op;
		buf.uGet(op);
		buf.uGetBytes(unkArrayFromLoginResponse,8);
		buf.uGet(unk1);
	}

	u8 unkArrayFromLoginResponse[8];
	u8 unk1;
};
