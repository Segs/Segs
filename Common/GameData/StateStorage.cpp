/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */
#include "StateStorage.h"

#include "Movement.h"
#include "GameData/CoHMath.h"

const char *control_name[] = {
    "FORWARD",
    "BACK",
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "PITCH",
    "YAW",
};

void TimeState::serializefrom_delta(BitStream &bs, const TimeState &/*prev*/)
{
    m_client_timenow = bs.GetPackedBits(1); // field_0 diff next-current
    m_time_res = bs.GetPackedBits(1);       // time to next state ?
    m_timestep = bs.GetFloat();             // next state's timestep

    m_time_rel1C = m_timestep;
    if(bs.GetBits(1))                       //timestep!=time_rel1C
        m_time_rel1C = bs.GetFloat();

    m_perf_cntr_diff = bs.Get64Bits();      //current_state->ticks - prev_state->ticks
    if(bs.GetBits(1))                       // perf freq changed between current and prev
        m_perf_freq_diff = bs.Get64Bits();
}

void TimeState::serializefrom_base(BitStream &bs)
{
    m_client_timenow   = bs.GetBits(32);    // result of time(NULL)
    m_time_res = bs.GetBits(32);            // result of timeGetTime()
    m_timestep = bs.GetFloat();             // client global TIMESTEP - per frame time

    m_time_rel1C = m_timestep;
    if(bs.GetBits(1))                       // timestep!=time_rel1C
        m_time_rel1C = bs.GetFloat();       // simulation timestep ?

    m_perf_cntr_diff = bs.Get64Bits();      // next_state->ticks - current_state->ticks
    m_perf_freq_diff = bs.Get64Bits();      // v7->perf_cntr1
}

void TimeState::dump()
{
    qCDebug(logInput, "CSC: %d,%d, [%f,%f]", m_client_timenow, m_time_res, m_timestep,m_time_rel1C);
    qCDebug(logInput, "(%lu %lu)", m_perf_cntr_diff, m_perf_freq_diff);
}

bool InputStateChange::hasInput() const
{
    if (m_has_keys)
    {
        for (bool key : m_keys)
        {
            if (key)
                return true;
        }
    }

    for (const ControlStateChange& csc : m_control_state_changes)
    {
        if (csc.control_id < 8) // key/mouse events are 0-7
            return true;
    }

    return false;
}

//! @}
