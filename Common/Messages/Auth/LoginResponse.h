/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cstring>
#include "Auth/AuthEvents.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class LoginResponse : public AuthLinkEvent
{
public:
    // [[ev_def:field]]
    uint8_t unkArray1[8];
    // [[ev_def:field]]
    uint8_t unkArray2[8];
    // [[ev_def:field]]
    uint32_t unk1=0;
    // [[ev_def:field]]
    uint32_t unk2=0;
    // [[ev_def:field]]
    uint32_t unk3=0;
    LoginResponse() : AuthLinkEvent(evLoginResponse)
    {
        memset(unkArray1,0,8);
        memset(unkArray2,0,8);
    }
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)3); // packet code
        buf.uPutBytes(unkArray1, sizeof(unkArray1));
        buf.uPutBytes(unkArray2, sizeof(unkArray2));
        buf.uPut(unk1);
        buf.uPut(unk2);
        buf.uPut(unk3);
    }
    void serializefrom(GrowingBuffer &buf) override
    {
        uint8_t op;
        buf.uGet(op);
        buf.uGetBytes(unkArray1, sizeof(unkArray1));
        buf.uGetBytes(unkArray2, sizeof(unkArray2));
        buf.uGet(unk1);
        buf.uGet(unk2);
        buf.uGet(unk3);
    }
    EVENT_IMPL(LoginResponse)
};
} //end of namespace SEGSEvents

