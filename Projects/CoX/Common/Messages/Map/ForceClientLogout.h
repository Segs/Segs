/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "BitStream.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class ForceLogout final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString reason;
    ForceLogout() :GameCommandEvent(MapEventTypes::evForceLogout) {}
    ForceLogout(const QString &_reason) :GameCommandEvent(MapEventTypes::evForceLogout),reason(_reason)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); //packet 12
        //bs.StorePackedBits(1,12); // opcode
        bs.StoreString(reason); // opcode
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(reason);
    }
    EVENT_IMPL(ForceLogout)
};

}
