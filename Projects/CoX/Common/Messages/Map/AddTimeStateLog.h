/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class AddTimeStateLog final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    int m_time_log = 0;

    explicit AddTimeStateLog() : GameCommandEvent(evAddTimeStateLog) {}
    AddTimeStateLog(int time_log) : GameCommandEvent(evAddTimeStateLog),
        m_time_log(time_log)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 60
        bs.StorePackedBits(1, m_time_log);
    }

    EVENT_IMPL(AddTimeStateLog)
};

} // end of SEGSEvents namespace
