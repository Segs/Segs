/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "NetStructures/LFG.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

class TeamLooking final : public GameCommand
{
public:
    uint32_t m_num = 0;
    std::vector<LFGMember> m_list;
    TeamLooking(std::vector<LFGMember> list) : GameCommand(MapEventTypes::evTeamLooking),
        m_num(list.size()),
        m_list(list)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 25

        bs.StoreBits(32,m_num); // size of list
        for(const auto &m : m_list)
        {
            bs.StoreString(m.m_name);
            bs.StoreString(m.m_classname);
            bs.StoreString(m.m_origin);
            bs.StoreBits(32,m.m_level);
        }

        if(logLFG().isDebugEnabled())
            dumpLFGList();
    }
    void    serializefrom(BitStream &src);
};
