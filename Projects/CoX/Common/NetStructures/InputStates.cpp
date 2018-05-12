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
#include "InputStates.h"
#include "Common/GameData/CoHMath.h"

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

/*
InputState &InputState::operator =(const InputState &other)
{
    m_csc_deltabits         = other.m_csc_deltabits;
    //m_send_deltas           = other.m_send_deltas;
    //m_control_bits          = other.m_control_bits;
    m_send_id               = other.m_send_id;
    m_time_diff1            = other.m_time_diff1;
    m_time_diff2            = other.m_time_diff2;
    m_has_historical_input  = other.m_has_historical_input;
    m_received_id           = other.m_received_id;
    m_no_collision          = other.m_no_collision;
    m_controls_disabled     = other.m_controls_disabled;

    for(int i=0; i<3; ++i)
    {
        if(other.m_pos_delta_valid[i])
            m_pos_delta[i] = other.m_pos_delta[i];
    }
    bool update_needed=false;
    for(int i=0; i<3; ++i)
    {
        if(other.m_pyr_valid[i])
            m_camera_pyr[i] = other.m_camera_pyr[i];

        if(other.m_orientation_pyr[i])
        {
            qCDebug(logOrientation) << other.m_orientation_pyr[i];
            m_orientation_pyr[i] = other.m_orientation_pyr[i];
            update_needed = true;
        }
    }
    if(update_needed)
        m_direction = fromCoHYpr(m_orientation_pyr);

    qCDebug(logOrientation) << m_direction.w << m_direction.x << m_direction.y << m_direction.z;
    return *this;
}
*/

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
    m_timestep = bs.GetFloat();             //client global TIMESTEP - per frame time

    m_time_rel1C = m_timestep;
    if(bs.GetBits(1))                       //timestep!=time_rel1C
        m_time_rel1C = bs.GetFloat();       // simulation timestep ?

    m_perf_cntr_diff = bs.Get64Bits();      //next_state->ticks - current_state->ticks
    m_perf_freq_diff = bs.Get64Bits();      //v7->perf_cntr1
}

void TimeState::dump()
{
    qCDebug(logInput, "CSC: %d,%d, [%f,%f]", m_client_timenow, m_time_res, m_timestep,m_time_rel1C);
    qCDebug(logInput, "(%lld %lld)", m_perf_cntr_diff, m_perf_freq_diff);
}

void StateStorage::addNewState(InputState &new_state)
{
    // Only update if we've actually received an update
    for(int i=0; i<3; ++i)
    {
        if(!new_state.m_pos_delta_valid[i])
            new_state.m_pos_delta[i] = m_inp_states.back().m_pos_delta[i];
    }
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
}

//! @}
