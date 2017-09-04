#pragma once
#include <cstring>
#include "AuthProtocol/AuthEvents.h"

class LoginResponse : public AuthLinkEvent
{
    uint8_t unkArray1[8];
    uint8_t unkArray2[8];
    uint32_t unk1, unk2, unk3;
public:
    LoginResponse() : AuthLinkEvent(evLoginResponse),unk1(0), unk2(0), unk3(0)
    {
        memset(unkArray1,0,8);
        memset(unkArray2,0,8);
    }
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut((uint8_t)3); // packet code
        buf.uPutBytes(unkArray1, sizeof(unkArray1));
        buf.uPutBytes(unkArray2, sizeof(unkArray2));
        buf.uPut(unk1);
        buf.uPut(unk2);
        buf.uPut(unk3);
    }
    void serializefrom(GrowingBuffer &buf)
    {
        uint8_t op;
        buf.uGet(op);
        buf.uGetBytes(unkArray1, sizeof(unkArray1));
        buf.uGetBytes(unkArray2, sizeof(unkArray2));
        buf.uGet(unk1);
        buf.uGet(unk2);
        buf.uGet(unk3);
    }
};
