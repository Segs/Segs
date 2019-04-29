/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Movement.h"

#include "Entity.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Common/GameData/CoHMath.h"
#include "Common/GameData/seq_definitions.h"
#include "Common/GameData/playerdata_definitions.h"
#include "Logging.h"

#include <glm/gtx/vector_query.hpp>
#include <glm/ext.hpp>
#include <chrono>

static const glm::mat3  s_identity_matrix = glm::mat3(1.0f);
static int          s_landed_on_ground = 0;
//static CollInfo     s_last_surf;
static const int s_reverse_control_dir[6] = {
    BinaryControl::BACKWARD,
    BinaryControl::FORWARD,
    BinaryControl::RIGHT,
    BinaryControl::LEFT,
    BinaryControl::DOWN,
    BinaryControl::UP,
};
static const char* s_key_name[6] =
{
    "FORWARD",
    "BACKWARD",
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
};

SurfaceParams g_world_surf_params[2] = {
    // traction, friction, bounce, gravity, max_speed
    //{ 1.00f, 0.45f, 0.01f, 0.065f, 1.00f }, // ground; from client (base to be modified later)
    //{ 0.02f, 0.01f, 0.00f, 0.065f, 1.00f },  // air; from client (base to be modified later)
    { 1.30f, 1.00f, 0.01f, 3.00f, 1.70f },      // ground; test values
    { 1.50f, 3.00f, 0.00f, 3.00f, 3.00f },      // air; test values
};

// how much is b newer than a
// e.g. 2: 2 changes ahead
// -3: 3 changes behind
static int32_t cscIdDelta(uint16_t a, uint16_t b)
{
    int32_t diff = b - a;
    if (diff > 0x8000)
    {
        return diff - 0xffff;
    }
    else if (diff < -0x7fff)
    {
        return diff + 0xffff; 
    }
    return diff;
}

void processNewInputs(Entity &e)
{
    // todo(jbr) sort out logging, maybe remove all logging from the packet parser and just log in here, input/movement?

    // state which resets every tick and is affected
    // by processing control state changes
    struct
    {
        uint32_t length_ms = 0;
        uint32_t key_press_start_ms[6] = {};
        bool    key_released[6] = {};
        bool    orientation_changed = false;
    } tick_state;

    StateStorage* input_state = &e.m_states;
    for (auto input_iter = input_state->m_new_inputs.begin();
         input_iter != input_state->m_new_inputs.end();
         ++input_iter) // todo(jbr) buffer these for jitter/cheating
    {
        const InputState& new_input = *input_iter;

        if (new_input.m_control_state_changes.size())
        {
            qCDebug(logInput, "csc range %hu->%hu", new_input.m_first_control_state_change_id, new_input.m_first_control_state_change_id + new_input.m_control_state_changes.size() - 1);

            // The client will re-send the same control state changes until acked, so
            // need to make sure we skip any changes we've seen before.
            int32_t csc_id_delta = cscIdDelta(new_input.m_first_control_state_change_id, input_state->m_next_expected_control_state_change_id);

            // if this is negative, it means there is a gap in control state changes, shouldn't ever happen
            assert(csc_id_delta >= 0);

            if (logInput().isDebugEnabled() && csc_id_delta > 0)
            {
                qCDebug(logInput, "skipping %d changes", csc_id_delta);
            }

            // if csc_id_delta >= control_state_changes.size() then all of 
            // these changes are old and should be ignored
            if (csc_id_delta < new_input.m_control_state_changes.size())
            {
                for (auto csc_iter = new_input.m_control_state_changes.begin() + csc_id_delta;
                    csc_iter != new_input.m_control_state_changes.end();
                    ++csc_iter)
                {
                    // todo(jbr) final pass on the decompiled src
                    const ControlStateChange& csc = *csc_iter;

                    tick_state.length_ms += csc.time_since_prev_ms;

                    switch (csc.control_id)
                    {
                        // Processing of keys based on inputFilterAndPassToSendList()
                        case BinaryControl::UP:
                        case BinaryControl::DOWN:
                        case BinaryControl::LEFT:
                        case BinaryControl::RIGHT:
                        case BinaryControl::FORWARD:
                        case BinaryControl::BACKWARD:
                        {
                            qCDebug(logInput, "key %hhu = %hhu", csc.control_id, csc.key_state);

                            uint8_t key = csc.control_id;

                            input_state->m_keys[key] = csc.key_state;

                            if (csc.key_state)
                            {
                                tick_state.key_press_start_ms[key] = tick_state.length_ms;
                            }
                            else
                            {
                                tick_state.key_released[key] = true;

                                if (!input_state->m_keys[s_reverse_control_dir[key]])
                                {
                                    uint32_t key_press_duration = tick_state.length_ms; // todo(jbr) not necessarily true if a key can be pressed and released same tick
                                    if (key_press_duration == 0)
                                    {
                                        if (input_state->m_key_press_duration_ms[key] == 0)
                                        {
                                            input_state->m_key_press_duration_ms[key] = 1;
                                        }
                                    }
                                    else
                                    {
                                        input_state->m_key_press_duration_ms[key] = std::min<uint32_t>(input_state->m_key_press_duration_ms[key] + key_press_duration, 1000);
                                    }
                                }
                            }
                        }
                        break;

                        case BinaryControl::PITCH:
                            qCDebug(logInput, "pitch=%f", csc.angle);

                            tick_state.orientation_changed = true;
                            e.m_entity_data.m_orientation_pyr.x = csc.angle;
                        break;

                        case BinaryControl::YAW:
                            qCDebug(logInput, "yaw=%f", csc.angle);

                            tick_state.orientation_changed = true;
                            e.m_entity_data.m_orientation_pyr.y = csc.angle;
                        break;

                        case 9:
                            qCDebug(logInput, "every_4_ticks=%hhu", csc.every_4_ticks);

                            input_state->m_every_4_ticks = csc.every_4_ticks;
                        break;

                        case 10:
                            qCDebug(logInput, "nocoll=%d", csc.no_collision);

                            if (csc.no_collision)
                            {
                                e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
                            }
                            else
                            {
                                e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;
                            }
                            e.m_motion_state.m_no_collision = csc.no_collision;
                        break;

                        case 8:
                            qCDebug(logInput, "doing tick, controls_disabled=%d, time_diff_1=%u, time_diff_2=%u, velocity_scale=%hhu", csc.controls_disabled, csc.time_diff_1, csc.time_diff_2, csc.velocity_scale);

                            // do a tick!

                            // Save current position to last_pos
                            e.m_motion_state.m_last_pos = e.m_entity_data.m_pos;

                            input_state->m_velocity_scale = csc.velocity_scale / 255.0f;

                            e.m_motion_state.m_controls_disabled = csc.controls_disabled;

                            // the following is based on pmotionUpdateControlsPrePhysics()

                            // if any keys have been held for 250+ ms, then all keys
                            // are given a minumum of 250ms press time
                            uint32_t minimum_key_press_time = 0;
                            if (e.m_motion_state.m_is_jumping)
                            {
                                minimum_key_press_time = 250;
                            }
                            else
                            {
                                for (int i = 0; i < 6; ++i)
                                {
                                    if (input_state->m_key_press_duration_ms[i] >= 250)
                                    {
                                        minimum_key_press_time = 250;
                                        break;
                                    }
                                }
                            }

                            for (int i = 0; i < 6; ++i)
                            {
                                if (!input_state->m_keys[i] || input_state->m_keys[s_reverse_control_dir[i]])
                                {
                                    continue;
                                }

                                // update key press time, keeping within min/max range
                                input_state->m_key_press_duration_ms[i] = glm::clamp<uint32_t>(input_state->m_key_press_duration_ms[i] + tick_state.length_ms - tick_state.key_press_start_ms[i],
                                           minimum_key_press_time, 1000);
                            }

                            // based on pmotionSetVel()
                            float control_amounts[6] = {};
                            for (int i = 0; i < 6; ++i)
                            {
                                uint32_t press_time = input_state->m_key_press_duration_ms[i];

                                if (!press_time)
                                {
                                    control_amounts[i] = 0.0f;
                                }
                                else if (press_time >= 1000)
                                {
                                    control_amounts[i] = 1.0f;
                                }
                                else if (press_time <= 50 && input_state->m_keys[i])
                                {
                                    control_amounts[i] = 0.0;
                                }
                                else if (press_time >= 75)
                                {
                                    if (press_time >= 100)
                                    {
                                        control_amounts[i] = (float)(press_time - 100) * 0.004f / 9.0f + 0.6f;
                                    }
                                    else
                                    {
                                        control_amounts[i] = std::pow((float)(press_time  - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
                                    }
                                }
                                else
                                {
                                    control_amounts[i] = 0.2f;
                                }
                            }

                            if (tick_state.orientation_changed)
                            {
                                // the control state change processing loop will have written pitch/yaw to
                                // entity data, so now update direction
                                e.m_direction = fromCoHYpr(e.m_entity_data.m_orientation_pyr);
                            }

                            glm::vec3 local_input_velocity(0.0f, 0.0f, 0.0f);
                            local_input_velocity.x = control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT];
                            local_input_velocity.y = control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN];
                            local_input_velocity.z = control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD];
                            local_input_velocity.x = local_input_velocity.x * e.m_motion_state.m_speed.x;
                            local_input_velocity.y = local_input_velocity.y * e.m_motion_state.m_speed.y;

                            glm::vec3 local_input_velocity_xz = local_input_velocity;
                            if (!e.m_motion_state.m_is_flying)
                            {
                                local_input_velocity_xz.y = 0;
                            }

                            float input_velocity_scale = input_state->m_velocity_scale;
                            if (local_input_velocity_xz.z < 0.0f)
                            {
                                input_velocity_scale = input_velocity_scale * e.m_motion_state.m_backup_spd;
                            }
                            if (e.m_motion_state.m_is_stunned)
                            {
                                input_velocity_scale = input_velocity_scale * 0.1f;
                            }
                            /*if (controls->speed_scale_F0 != 0.0f) todo(jbr)
                            {
                                input_velocity_scale *= controls->speed_scale_F0;
                            }*/
                            e.m_motion_state.m_velocity_scale = input_velocity_scale;

                            if (glm::length2(local_input_velocity_xz) > glm::epsilon<float>())
                            {
                                local_input_velocity_xz = glm::normalize(local_input_velocity_xz);
                            }

                            local_input_velocity.x = local_input_velocity_xz.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
                            local_input_velocity.z = local_input_velocity_xz.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);
                            if (e.m_motion_state.m_is_flying)
                            {
                                local_input_velocity.y = local_input_velocity_xz.y * std::fabs(control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN]);
                            }
                            else if (tick_state.key_released[BinaryControl::UP])
                            {
                                local_input_velocity.y = 0;
                            }
                            else
                            {
                                local_input_velocity.y *= glm::clamp<float>(e.m_motion_state.m_jump_height, 0.0f, 1.0f);

                                if (!e.m_motion_state.m_is_sliding)
                                {
                                    e.m_motion_state.m_flag_5 = false;
                                }
                            }

                            glm::vec3 input_velocity = e.m_direction * local_input_velocity;

                            e.m_motion_state.m_input_velocity = input_velocity;

                            // based on pmotionWithPrediction()
                            float timestep = 1.0f; // todo(jbr) can this change?

                            // todo(jbr) check move type is nocoll here, put unimplemented block for all else
                            // based on entMoveNoCollision()
                            e.m_motion_state.m_is_falling = true;
                            e.m_motion_state.m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                            e.m_motion_state.m_move_time += timestep;
                            float time_scale = (e.m_motion_state.m_move_time + 6.0f) / 6.0f;
                            time_scale = std::min(time_scale, 50.0f);

                            e.m_entity_data.m_pos += time_scale * timestep * input_velocity;

                            if (glm::length2(local_input_velocity) < glm::epsilon<float>())
                            {
                                e.m_motion_state.m_move_time = 0.0f;
                            }

                            if (logMovement().isDebugEnabled())
                            {
                                static int32_t count = -1;

                                bool any_keys_relevant = false;
                                for (int i = 0; i < 6; ++i)
                                {
                                    if (input_state->m_key_press_duration_ms[i])
                                    {
                                        any_keys_relevant = true;
                                        break;
                                    }
                                }

                                if (!any_keys_relevant && e.m_motion_state.m_last_pos == e.m_entity_data.m_pos)
                                {
                                    count = -1;
                                }
                                else
                                {
                                    ++count;

                                    QString keys = "";
                                    for (int i = 0; i < 6; ++i)
                                    {
                                        if (input_state->m_key_press_duration_ms[i])
                                        {
                                            QTextStream(&keys) << (tick_state.key_released[i] ? "-" : "+") << s_key_name[i] << " (" << input_state->m_key_press_duration_ms[i] << "), ";
                                        }
                                    }

                                    qCDebug(logMovement,
                                            "\n"
                                            "%4d.    keys:      %s\n"
                                            "        pos:       (%1.8f, %1.8f, %1.8f)\n"
                                            "      + vel:       (%1.8f, %1.8f, %1.8f)\n"
                                            "      + inpvel:    (%1.8f, %1.8f, %1.8f) @ %f\n"
                                            "      + pyr:       (%1.8f, %1.8f, %1.8f)\n"
                                            "      + misc:      grav=%1.3f, %s%s\n"
                                            "      + move_time: %1.3f\n"
                                            "      = newpos:    (%1.8f, %1.8f, %1.8f)\n"
                                            "        newvel:    (%1.8f, %1.8f, %1.8f)\n"
                                            "%s%s\n",
                                            count,
                                            keys.data(),
                                            e.m_motion_state.m_last_pos.x, e.m_motion_state.m_last_pos.y, e.m_motion_state.m_last_pos.z,
                                            e.m_motion_state.m_velocity.x, e.m_motion_state.m_velocity.y, e.m_motion_state.m_velocity.z,
                                            e.m_motion_state.m_input_velocity.x, e.m_motion_state.m_input_velocity.y, e.m_motion_state.m_input_velocity.z,
                                            e.m_entity_data.m_orientation_pyr.x, e.m_entity_data.m_orientation_pyr.y, e.m_entity_data.m_orientation_pyr.z,
                                            0.0f, // gravity
                                            e.m_motion_state.m_is_jumping ? "Jumping, " : "",
                                            e.m_motion_state.m_controls_disabled ? "NoControls, " : "",
                                            e.m_motion_state.m_move_time,
                                            e.m_entity_data.m_pos.x, e.m_entity_data.m_pos.y, e.m_entity_data.m_pos.z,
                                            e.m_motion_state.m_velocity.x, e.m_motion_state.m_velocity.y, e.m_motion_state.m_velocity.z,
                                            e.m_motion_state.m_no_collision ? "    ** NO ENT COLL **\n" : "",
                                            e.m_motion_state.m_is_flying ? "    ** FLYING **\n" : "");
                                }
                            }

                            // based on pmotionResetMoveTime()
                            for (int i = 0; i < 6; ++i)
                            {
                                // reset total time if button is released, or reverse button is pressed
                                if (!input_state->m_keys[i] || input_state->m_keys[s_reverse_control_dir[i]])
                                {
                                    input_state->m_key_press_duration_ms[i] = 0;
                                }
                            }

                            tick_state = {};

                        break;
                    }
                }
                // todo(jbr) warnings
                uint16_t new_csc_count = new_input.m_control_state_changes.size() - csc_id_delta;
                qCDebug(logInput, "processed csc %hu->%hu", input_state->m_next_expected_control_state_change_id, input_state->m_next_expected_control_state_change_id + new_csc_count - 1);
                input_state->m_next_expected_control_state_change_id += new_csc_count;
            }
        }

        if (new_input.m_has_keys)
        {
            for (int i = 0; i < 6; ++i)
            {
                if (new_input.m_keys[i] != input_state->m_keys[i])
                {
                    qCDebug(logInput, "keys input state mismatch");
                    input_state->m_keys[i] = new_input.m_keys[i];
                }
            }
        }

        if (logInput().isDebugEnabled() && new_input.m_has_pitch_and_yaw)
        {
            qCDebug(logInput, "extended pitch=%f, yaw=%f", new_input.m_pitch, new_input.m_yaw);
        }
    }

    input_state->m_new_inputs.clear();
}

void addPosUpdate(Entity &e, const PosUpdate &p)
{
    e.m_update_idx = (e.m_update_idx+1) % 64;
    e.m_pos_updates[e.m_update_idx] = p;
}

void forcePosition(Entity &e, glm::vec3 pos)
{
    e.m_entity_data.m_pos = pos;
    e.m_force_pos_and_cam = true;
}

void forceOrientation(Entity &e, glm::vec3 pyr)
{
    e.m_direction = glm::quat(pyr);
    e.m_entity_data.m_orientation_pyr = pyr;
    e.m_force_pos_and_cam = true;
}

// Move to Sequences or Triggers files later
void addTriggeredMove(Entity &e, uint32_t move_idx, uint32_t delay, uint32_t fx_idx)
{
    e.m_update_anims = true;
    e.m_rare_update = true;

    TriggeredMove tmove;
    tmove.m_move_idx = move_idx;
    tmove.m_ticks_to_delay = delay;
    tmove.m_trigger_fx_idx = fx_idx;

    e.m_triggered_moves.push_back(tmove);
    qCDebug(logAnimations) << "Queueing triggered move:"
                           << move_idx << delay << fx_idx;
}

//! @}
