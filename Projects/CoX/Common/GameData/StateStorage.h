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

struct ControlStateChangesForTick
{
    struct KeyChange
    {
        int key = 0;
        int state = 0;
        uint32_t offset_from_tick_start_ms = 0;

        template<class Archive>
        void serialize(Archive &ar)
        {
            ar(key);
            ar(state);
            ar(offset_from_tick_start_ms);
        }
    };

    uint16_t first_change_id        = 0xffff;
    uint16_t last_change_id         = 0xffff;
    uint32_t tick_length_ms         = 0;
    std::vector<KeyChange> key_changes;
    bool pitch_changed              = false;
    float pitch                     = 0.0f;
    bool yaw_changed                = false;
    float yaw                       = 0.0f;
    bool no_collision_changed       = false;
    bool no_collision               = false;
    bool controls_disabled          = false; // todo(jbr)
    uint32_t time_diff_1            = 0;
    uint32_t time_diff_2            = 0;
    uint8_t velocity_scale          = 0;
    bool every_4_ticks_changed      = false; // what's this actually for?
    uint8_t every_4_ticks           = 0;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(first_change_id);
        ar(last_change_id);
        ar(tick_length_ms);
        ar(key_changes);
        ar(pitch_changed);
        ar(pitch);
        ar(yaw_changed);
        ar(yaw);
        ar(no_collision_changed);
        ar(no_collision);
        ar(controls_disabled);
        ar(time_diff_1);
        ar(time_diff_2);
        ar(velocity_scale);
        ar(every_4_ticks_changed);
        ar(every_4_ticks);
    }
};
// is a newer than b?
bool isControlStateChangeIdNewer(uint16_t a, uint16_t b); // todo(jbr) think this can go

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

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_client_timenow);
        ar(m_time_res);
        ar(m_timestep);
        ar(m_time_rel1C);
        ar(m_perf_cntr_diff);
        ar(m_perf_freq_diff);
    }
};



class InputState
{
public:
    // todo(jbr) document all these, what do they mean, when are they sent, etc

    std::vector<ControlStateChangesForTick> m_control_state_changes;
    bool m_has_keys = false;
    bool m_keys[6] = {};
    bool m_has_pitch_and_yaw = false;
    float m_pitch = 0.0f; // todo(jbr) check, do these match whatever we got in control state changes?
    float m_yaw = 0.0f;
    bool m_has_target = false;
    uint32_t    m_target_idx = 0;
    std::vector<TimeState> m_time_state;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_control_state_changes);
        ar(m_has_keys);
        ar(m_keys);
        ar(m_has_pitch_and_yaw);
        ar(m_pitch);
        ar(m_yaw);
        ar(m_has_target);
        ar(m_target_idx);
        ar(m_time_state);
    }

    bool hasInput() const;

/*    using FloatDuration = std::chrono::duration<float>;
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

    QVector<ControlStateChangesForTick> m_control_state_changes;

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
    }*/
};

class StateStorage
{
public:
    QVector<InputState> m_inp_states;

    uint16_t m_next_expected_control_state_change_id = 0;

    bool m_keys[6] = {};
    uint32_t m_key_press_duration_ms[6] = {};

    void addNewState(InputState &new_state);
};
