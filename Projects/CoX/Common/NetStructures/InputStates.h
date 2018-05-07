/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "CommonNetStructures.h"
#include "Logging.h"

#include <glm/gtx/quaternion.hpp>

enum BinaryControl
{
    FORWARD     = 0,
    BACKWARD    = 1,
    LEFT        = 2,
    RIGHT       = 3,
    UP          = 4,
    DOWN        = 5,
    PITCH       = 6,
    YAW         = 7,
    LAST_BINARY_VALUE = 5,
    LAST_QUANTIZED_VALUE = 7,
};

extern const char *control_name[];

struct SurfaceParams
{
    float traction;
    float friction;
    float bounce;
    float gravitational_constant;
    float max_speed;
};

class InputState
{
public:
    InputState()
    {
        for(int i=0; i<3; ++i)
        {
            pos_delta_valid[i]=false;
            pyr_valid[i]=false;
        }
    }

    uint8_t     m_csc_deltabits         = 0;
    uint16_t    m_send_id               = 0;
    uint8_t     m_received_id           = 0;

    bool        m_autorun               = 0;    // send_bits? autorun?
    uint16_t    m_control_bits[6]       = {0};
    uint16_t    m_prev_control_bits[6]  = {0};  // maybe goes away?
    glm::vec3   m_camera_pyr;
    glm::vec3   m_orientation_pyr;      // Stored in Radians
    glm::quat   m_direction;
    int         m_time_diff1            = 0;
    int         m_time_diff2            = 0;
    float       m_velocity_scale        = 1.0f;
    float       m_speed_scale           = 1.0f;
    bool        m_no_collision          = false;
    bool        m_has_historical_input  = 0;
    bool        pos_delta_valid[3]      = {};
    bool        pyr_valid[3]            = {};
    glm::vec3   pos_delta;
    bool        m_controls_disabled     = false;
    float       m_keypress_time[6]      = {0};

    // Targeting
    bool        m_has_target;
    uint32_t    m_target_idx;
    uint32_t    m_assist_target_idx;

};

class TimeState
{
public:
    int         m_client_timenow;
    int         m_time_res;
    float       m_timestep;
    float       m_time_rel1C;
    uint64_t    m_perf_cntr_diff;
    uint64_t    m_perf_freq_diff;

    // recover actual ControlState from network data and previous entry
    void serializefrom_delta(BitStream &bs, const TimeState &prev);
    void serializefrom_base(BitStream &bs);
    void dump();
};

class StateStorage
{
public:
    std::vector<InputState> m_inp_states;
    // TODO: maybe move each of these other states inside each InputState?
    std::vector<TimeState>  m_time_states;
    // std::vector<SpeedState>  m_speed_states;
    // std::vector<MotionState>  m_motion_states;

    void addNewState(const InputState &new_state);
};
