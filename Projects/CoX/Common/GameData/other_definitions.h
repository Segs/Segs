/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <vector>
#include <stdint.h>

struct Parse_Combining
{
    std::vector<float> CombineChances;
};

struct Parse_Effectiveness
{
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
