/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
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
    qCDebug(logInput, "(%lld %lld)", m_perf_cntr_diff, m_perf_freq_diff);
}

void StateStorage::addNewState(InputState &new_state)
{
    if(m_inp_states.size() < 1)
        m_inp_states.push_back(new_state);

    //new_state.m_pos_start = m_inp_states.back().m_pos_end;
    //new_state.m_pos_delta = m_inp_states.back().m_pos_end;

    bool update_needed=false;
    for(int i=0; i<3; ++i)
    {
        if(!new_state.m_pyr_valid[i])
            new_state.m_camera_pyr[i] = m_inp_states.back().m_camera_pyr[i];

        if(new_state.m_orientation_pyr[i] != m_inp_states.back().m_orientation_pyr[i])
            update_needed = true;
    }

    if(update_needed)
        new_state.m_direction = fromCoHYpr(new_state.m_orientation_pyr);

    m_inp_states.push_back(new_state);
    // m_time_states.push_back(new_time_state);
    // m_speed_states.push_back(new_speed_state);
    // m_motion_states.push_back(new_motion_state);

    if(m_inp_states.size() > 30)
        m_inp_states.pop_front();
}

//! @}
