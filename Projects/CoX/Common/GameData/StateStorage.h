/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "CommonNetStructures.h"
#include "Logging.h"
#include <glm/gtc/quaternion.hpp> // for quat

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

class TimeState
{
public:
    int         m_client_timenow    = 0;
    int         m_time_res          = 0;
    float       m_timestep          = 0.0f;
    float       m_time_rel1C        = 0.0f;
    uint64_t    m_perf_cntr_diff    = 0;
    uint64_t    m_perf_freq_diff    = 0;

    // recover actual ControlState from network data and previous entry
    void serializefrom_delta(BitStream &bs, const TimeState &prev);
    void serializefrom_base(BitStream &bs);
    void dump();
};

class InputState
{
    using FloatDuration = std::chrono::duration<float>;
public:
    InputState()
    {
        for(int i=0; i<3; ++i)
        {
            m_pos_delta_valid[i]    = false;
            m_pyr_valid[i]          = false;
        }
    }

    uint8_t     m_csc_deltabits         = 0;
    uint16_t    m_send_id               = 0;
    uint8_t     m_every_4_ticks         = 0;

    bool        m_full_timeupdate       = 0;    // send_bits? autorun?

    uint8_t     m_updated_bit_pos       = 7;    // 7 is full update
    uint16_t    m_control_bits[6]       = {0};
    glm::vec3   m_camera_pyr            = {0.0f, 0.0f, 0.0f};
    glm::vec3   m_orientation_pyr;      // Stored in Radians
    glm::quat   m_direction;
    int         m_time_diff1            = 0;
    int         m_time_diff2            = 0;
    float       m_velocity_scale        = 1.0f;
    float       m_speed_scale           = 1.0f;
    bool        m_no_collision          = false;
    bool        m_controls_disabled     = false;
    bool        m_full_input_packet     = 0;
    bool        m_input_received        = false;
    bool        m_pos_delta_valid[3]    = {false};
    bool        m_pyr_valid[3]          = {false};
    glm::vec3   m_pos_delta             = {0.0f, 0.0f, 0.0f};
    std::chrono::steady_clock::time_point m_keypress_start[6];
    FloatDuration m_svr_keypress_time[6]; // for debugging
    float       m_keypress_time[6]      = {0};  // total_move
    float       m_move_time             = 0.0f;
    float       m_ms_since_prev;
    bool        m_following             = false;
    glm::vec3   m_pos_start             = {0.0f, 0.0f, 0.0f};
    glm::vec3   m_pos_end               = {0.0f, 0.0f, 0.0f};
    int         m_landing_recovery_time = {0};

    // Targeting
    bool        m_has_target;
    uint32_t    m_target_idx;
    uint32_t    m_assist_target_idx;

    TimeState   m_time_state;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_csc_deltabits);
        ar(m_control_bits);
        ar(m_send_id);
        ar(m_camera_pyr);
        ar(m_orientation_pyr);
        ar(m_direction);
        ar(m_time_diff1);
        ar(m_time_diff2);
        ar(m_velocity_scale);
        ar(m_every_4_ticks);
        ar(m_no_collision);
        ar(m_full_input_packet);
        ar(m_pos_delta_valid);
        ar(m_pyr_valid);
        ar(m_pos_delta);
        ar(m_controls_disabled);
    }
};

class StateStorage
{
public:
    QVector<InputState> m_inp_states;
    int m_oldest_unprocessed_input_index = 0;
    // TODO: maybe move these other states here and vectorize StateStorage?
    // std::vector<TimeState>  m_time_states;
    // std::vector<SpeedState>  m_speed_states;
    // std::vector<MotionState>  m_motion_states;

    void init()
    {
        InputState empty_state;
        // Fill with empty state for previous()
        addNewState(empty_state);
    }

    InputState* current() { return &m_inp_states.back(); }
    const InputState* current() const { return &m_inp_states.back(); }
    InputState* previous()
    {
        if(m_inp_states.size() < 2)
            return &m_inp_states.back();

        return &m_inp_states.back()-1;
    }
    const InputState* previous() const
    {
        if(m_inp_states.size() < 2)
            return &m_inp_states.back();

        return &m_inp_states.back()-1;
    }
    InputState* getOldestUnprocessedInput();

    void addNewState(InputState &new_state);
};
