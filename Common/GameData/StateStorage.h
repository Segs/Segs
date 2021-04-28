/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "Components/Logging.h"
#include <glm/gtc/quaternion.hpp> // for quat

class BitStream;

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
      uint8_t key_state;            // control_id 0-5
      float angle;                  // control_id 6-7
      struct
      {
          bool controls_disabled;   // control_id 8
          uint32_t time_diff_1;
          uint32_t time_diff_2;
          uint8_t velocity_scale;
      } control_id_8;
      uint8_t every_4_ticks;        // control_id 9 (not sure what this is for)
      bool no_collision;            // control_id 10
    } data;

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
                ar(data.key_state);
                break;

            case BinaryControl::PITCH:
            case BinaryControl::YAW:
                ar(data.angle);
                break;

            case 8:
                ar(data.control_id_8.controls_disabled);
                ar(data.control_id_8.time_diff_1);
                ar(data.control_id_8.time_diff_2);
                ar(data.control_id_8.velocity_scale);
                break;

            case 9:
                ar(data.every_4_ticks);
                break;

            case 10:
                ar(data.no_collision);
                break;

            default:
                assert(!"unexpected control id");
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

class InputStateChange
{
public:
    /* control state changes
        - can be empty
        - can contain multiple ticks
        - doesn't contain partial ticks
        - may contain ticks which have been received before by server (client resends until acked)
    */
    std::vector<ControlStateChange> m_control_state_changes;

    // id of first control state change in the vector, each change bumps the id
    // by 1, NOT the tick, wraps around to 0 on overflow. Use these to figure
    // out if these control state changes are new or not.
    uint16_t    m_first_control_state_change_id = 0xffff;

    // whether this state change contains the keys currently held in m_keys,
    // doesn't necessarily mean any of them have changed, but useful for 
    // validating against current key state on server
    bool        m_has_keys = false;
    bool        m_keys[6] = {};

    // whether this state change contains the client pitch/yaw which are sent
    // every second. This seems to deviate slightly from the pitch/yaw changes
    // which are sent in the control state changes, so maybe it's camera pitch/yaw?
    bool        m_has_pitch_and_yaw = false;
    float       m_pitch = 0.0f;
    float       m_yaw = 0.0f;

    // whether this state change contains a new target
    bool        m_has_target = false;
    uint32_t    m_target_idx = 0;

    // not sure what these do
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

class InputState
{
public:
    // input changes which have been received from the client but not processed yet
    std::vector<InputStateChange>   m_queued_changes;

    // next control state change id that the server wants, used to avoid processing
    // the same control state changes multiple times
    uint16_t                        m_next_expected_control_state_change_id = 0;

    // keys currently held
    bool                            m_keys[6] = {};

    // how long each key has been held
    uint32_t                        m_key_press_duration_ms[6] = {};

    // sent by client, alters speed of all movement
    float                           m_velocity_scale = 1.0f;

    // sent by client, not sure what it's for
    uint8_t                         m_every_4_ticks = 0;

    // seems to only ever be 1 on the client when predicting movement, so should only need to be 1 on server (I think) todo(jbr)
    float                           m_timestep = 1.0f; // todo(jbr) is this in the packet? the array of timestate things, equal to number of full ticks?

    // whether to generate input logging for this entity
    bool                            m_debug = false;
};
