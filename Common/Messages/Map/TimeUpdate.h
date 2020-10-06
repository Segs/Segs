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

namespace SEGSEvents
{

// [[ev_def:type]]
class TimeUpdate final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    int32_t m_time_since_jan_1_2000 = 0;

    explicit TimeUpdate() : GameCommandEvent(evTimeUpdate) {}
    TimeUpdate(int32_t time) : GameCommandEvent(evTimeUpdate),
        m_time_since_jan_1_2000(time)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 48
        bs.StorePackedBits(32, m_time_since_jan_1_2000);
    }

    EVENT_IMPL(TimeUpdate)
};

} // end of SEGSEvents namespace
