#pragma once
#include "AuthEvents.h"

class ServerListRequest : public AuthLinkEvent
{
public:
	ServerListRequest() : AuthLinkEvent(evServerListRequest),unk1(0xFF)
	{}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((uint8_t)5);
		buf.uPutBytes(unkArrayFromLoginResponse, sizeof(unkArrayFromLoginResponse));
		buf.uPut(unk1);	
	}
	void serializefrom(GrowingBuffer &buf)
	{
		uint8_t op;
		buf.uGet(op);
		buf.uGetBytes(unkArrayFromLoginResponse,8);
		buf.uGet(unk1);
	}

	uint8_t unkArrayFromLoginResponse[8];
	uint8_t unk1;
};
