/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "AuthProtocol/AuthEvents.h"

struct LoginRequestData
{
    char login[14];
    char password[16];
    uint32_t unkval1=0;
    uint16_t unkval2=0;
};
class LoginRequest : public AuthLinkEvent
{
public:
    LoginRequest() : AuthLinkEvent(evLogin)
    {}
    void serializeto(GrowingBuffer &buf) const
    {
        buf.uPut(uint8_t(0));
        buf.uPutBytes((uint8_t*)m_data.login, sizeof(m_data.login));
        buf.uPutBytes((uint8_t*)m_data.password, sizeof(m_data.password));
        buf.uPut(m_data.unkval1);
        buf.uPut(m_data.unkval2);
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
        buf.uGetBytes((uint8_t *)m_data.login, sizeof(m_data.login));
        buf.uGetBytes((uint8_t *)m_data.password, sizeof(m_data.password));
        buf.uGet(m_data.unkval1);
        buf.uGet(m_data.unkval2);
    }
    LoginRequestData m_data;
};
