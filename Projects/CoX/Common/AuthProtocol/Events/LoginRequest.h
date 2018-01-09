#pragma once
#include "AuthProtocol/AuthEvents.h"

class LoginRequest : public AuthLinkEvent
{
public:
    LoginRequest() : AuthLinkEvent(evLogin),unkval1(0),unkval2(0)
    {}
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut(uint8_t(0));
        buf.uPutBytes((uint8_t*)login, sizeof(login));
        buf.uPutBytes((uint8_t*)password, sizeof(password));
        buf.uPut(unkval1);
        buf.uPut(unkval2);
        //assert(!"Not implemented");
    }
    void serializefrom(GrowingBuffer &buf)
    {
        uint8_t packet_code;
        buf.uGet(packet_code);
        if(packet_code!=0)
        {
            //assert(packet_code==0);
        }
        buf.uGetBytes((uint8_t *)login, sizeof(login));
        buf.uGetBytes((uint8_t *)password, sizeof(password));
        buf.uGet(unkval1);
        buf.uGet(unkval2);
    }
    char login[14];
    char password[16];
    uint32_t unkval1;
    uint16_t unkval2;
};
