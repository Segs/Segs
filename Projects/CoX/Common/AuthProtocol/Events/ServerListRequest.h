/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "AuthProtocol/AuthEvents.h"

// [[ev_def:type]]
class ServerListRequest : public AuthLinkEvent
{
public:
    ServerListRequest() : AuthLinkEvent(evServerListRequest)
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

    // [[ev_def:field]]
    uint8_t unkArrayFromLoginResponse[8];
    // [[ev_def:field]]
    uint8_t unk1=0xFF;
};
