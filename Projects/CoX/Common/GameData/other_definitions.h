/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "Common/GameData/attrib_definitions.h"

#include <vector>
#include <stdint.h>

struct Parse_Combining {
    std::vector<float> CombineChances;
};

struct Parse_Effectiveness {
    std::vector<float> Effectiveness;
};

struct LevelExpAndDebt
{
    std::vector<uint32_t> m_ExperienceRequired;
    std::vector<uint32_t> m_DefeatPenalty;
};

struct Parse_PI_Schedule
{
    std::vector<uint32_t> m_FreeBoostSlotsOnPower;
    std::vector<uint32_t> m_PoolPowerSet;
    std::vector<uint32_t> m_Power;
    std::vector<uint32_t> m_AssignableBoost;
    std::vector<uint32_t> m_InspirationCol;
    std::vector<uint32_t> m_InspirationRow;
    std::vector<uint32_t> m_BoostSlot;
};
