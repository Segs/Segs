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

struct ControlStateChange
{
    uint8_t control_id = 0xff;
    uint32_t time_since_prev_ms = 0;

    union
    {
      uint8_t key_state;
      float angle;
      struct
      {
          bool controls_disabled;
          uint32_t time_diff_1;
          uint32_t time_diff_2;
          uint8_t velocity_scale;
      };
      uint8_t every_4_ticks; // todo(jbr) what's this actually for?
      bool no_collision;
    };

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(control_id);
        ar(time_since_prev_ms);

        switch (control_id)
        {
            case BinaryControl::FORWARD:
            case BinaryControl::BACKWARD:
            case BinaryControl::LEFT:
            case BinaryControl::RIGHT:
            case BinaryControl::UP:
            case BinaryControl::DOWN:
                ar(key_state);
                break;

            case BinaryControl::PITCH:
            case BinaryControl::YAW:
                ar(angle);
                break;

            case 8:
                ar(controls_disabled);
                ar(time_diff_1);
                ar(time_diff_2);
                ar(velocity_scale);
                break;

            case 9:
                ar(every_4_ticks);
                break;

            case 10:
                ar(no_collision);
                break;

            default:
                assert("unexpected control id");
                break;
        }
    }
};

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

    std::vector<ControlStateChange> m_control_state_changes;
    uint16_t m_first_control_state_change_id = 0xffff;
    bool m_has_keys = false;
    bool m_keys[6] = {};
    bool m_has_pitch_and_yaw = false;
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;
    bool m_has_target = false;
    uint32_t    m_target_idx = 0;
    std::vector<TimeState> m_time_state;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_control_state_changes);
        ar(m_first_control_state_change_id);
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
};

class StateStorage // todo(jbr) maybe call this InputState?
{
public:
    std::vector<InputState> m_new_inputs;
    uint16_t m_next_expected_control_state_change_id = 0;
    bool m_keys[6] = {};
    uint32_t m_key_press_duration_ms[6] = {};
    float m_velocity_scale = 1.0f;
    uint8_t m_every_4_ticks = 0;
};
