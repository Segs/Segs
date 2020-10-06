/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class LevelUp final : public GameCommandEvent
{
public:
        LevelUp() : GameCommandEvent(evLevelUp)
        {
        }

        void    serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // pkt 61

            // nothing to send
        }
        EVENT_IMPL(LevelUp)
};

// [[ev_def:type]]
class LevelUpResponse final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t button_id;
    // [[ev_def:field]]
    uint32_t result;

    LevelUpResponse():MapLinkEvent(evLevelUpResponse)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 47
    }
    void    serializefrom(BitStream &/*bs*/) override
    {
        qCDebug(logMapEvents) << "LevelUp Complete";

        //button_id = bs.GetPackedBits(1);
        //result = bs.GetPackedBits(1);
    }

    EVENT_IMPL(LevelUpResponse)
};

} // end of SEGSEvents namespace
