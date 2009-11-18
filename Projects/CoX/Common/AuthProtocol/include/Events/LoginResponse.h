#pragma once
#include "AuthEvents.h"

class LoginResponse : public AuthLinkEvent
{
	u8 unkArray1[8];
	u8 unkArray2[8];
	u32 unk1, unk2, unk3;
public:
	LoginResponse() : AuthLinkEvent(evLoginResponse),unk1(0), unk2(0), unk3(0)
	{}
	void serializeto(GrowingBuffer &buf) const
	{
		buf.uPut((u8)3); // packet code
		buf.uPutBytes(unkArray1, sizeof(unkArray1));
		buf.uPutBytes(unkArray2, sizeof(unkArray2));
		buf.uPut(unk1);
		buf.uPut(unk2);
		buf.uPut(unk3);
	}
	void serializefrom(GrowingBuffer &buf)
	{
		u8 op;
		buf.uGet(op);
		buf.uGetBytes(unkArray1, sizeof(unkArray1));
		buf.uGetBytes(unkArray2, sizeof(unkArray2));
		buf.uGet(unk1);
		buf.uGet(unk2);
		buf.uGet(unk3);
	};
};
