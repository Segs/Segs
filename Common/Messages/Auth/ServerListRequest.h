/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Auth/AuthEvents.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class ServerListRequest : public AuthLinkEvent
{
public:
    ServerListRequest() : AuthLinkEvent(evServerListRequest)
    {}
    void serializeto(GrowingBuffer &buf) const override
    {
        buf.uPut((uint8_t)5);
        buf.uPutBytes(unkArrayFromLoginResponse, sizeof(unkArrayFromLoginResponse));
        buf.uPut(unk1);
    }
    void serializefrom(GrowingBuffer &buf) override
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
    EVENT_IMPL(ServerListRequest)
};
} // end of namespace SEGSEvents
