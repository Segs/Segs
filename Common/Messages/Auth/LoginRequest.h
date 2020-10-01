/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Auth/AuthEvents.h"
#include <array>

struct LoginRequestData
{
    std::array<char,14> login;
    std::array<char,16> password;
    uint32_t unkval1=0;
    uint16_t unkval2=0;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( login, password, unkval1,unkval2 );
    }
};
namespace SEGSEvents
{
// [[ev_def:type]]
class LoginRequest : public AuthLinkEvent
{
public:
    LoginRequest() : AuthLinkEvent(evLoginRequest)
    {}
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut(uint8_t(0));
        buf.uPutBytes((uint8_t*)m_data.login.data(), m_data.login.size());
        buf.uPutBytes((uint8_t*)m_data.password.data(), m_data.password.size());
        buf.uPut(m_data.unkval1);
        buf.uPut(m_data.unkval2);
        //assert(!"Not implemented");
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t packet_code;
        buf.uGet(packet_code);
        if(packet_code!=0)
        {
            //assert(packet_code==0);
        }
        buf.uGetBytes((uint8_t *)m_data.login.data(), m_data.login.size());
        buf.uGetBytes((uint8_t *)m_data.password.data(), m_data.password.size());
        buf.uGet(m_data.unkval1);
        buf.uGet(m_data.unkval2);
    }
    // [[ev_def:field]]
    LoginRequestData m_data;
    EVENT_IMPL(LoginRequest)
};
} // end of namespace SEGSEvents
