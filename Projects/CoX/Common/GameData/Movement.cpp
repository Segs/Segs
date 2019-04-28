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

        uint16_t csc_id = new_input.m_first_control_state_change_id;
        for (auto csc_iter = new_input.m_control_state_changes.begin();
             csc_iter != new_input.m_control_state_changes.end();
             ++csc_iter, ++csc_id)
        {
            // todo(jbr) final pass on the decompiled src
            // The client will re-send the same control state changes until acked, so
            // need to make sure we ignore any changes we've seen before. Also, need
            // to use != rather than <, as the ids will wrap around to zero again.
            if (csc_id != input_state->m_next_expected_control_state_change_id)
            {
                continue;
            }
            ++input_state->m_next_expected_control_state_change_id;

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
                    tick_state.orientation_changed = true;
                    e.m_entity_data.m_orientation_pyr.x = csc.angle;
                break;

                case BinaryControl::YAW:
                    tick_state.orientation_changed = true;
                    e.m_entity_data.m_orientation_pyr.y = csc.angle;
                break;

                case 9:
                    // todo(jbr) every 4 ticks
                // todo(jbr) log csc?
                    input_state->m_every_4_ticks = csc.every_4_ticks;
                break;

                case 10:
                // todo(jbr) log csc?
                    if (csc.no_collision)
                    {
                        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
                    }
                    else
                    {
                        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;
                    }
                break;

                case 8:
                    // todo(jbr) log csc?

                    // do a tick!

                    // Save current position to last_pos
                    e.m_motion_state.m_last_pos = e.m_entity_data.m_pos;

                    input_state->m_velocity_scale = csc.velocity_scale / 255.0f; // todo(jbr) check what we're actually getting for vscale

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

                    // todo(jbr) command to enable movement logging?
                    /*for (int i = 0; i < 6; ++i)
                    {
                        if (input_state->m_key_press_time_ms[i])
                        {
                            qCDebug(logMovement, "%s%s (%dms)", input_state->m_keys[i] ? "+" : "-", s_key_name[i], input_state->m_key_press_time_ms[i]);
                        }
                    }*/

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
                        // todo(jbr) command to enable movement logging?
                        //qCDebug(logMovement, "new pyr = (%f, %f, %f)", orientation_pyr.p, orientation_pyr.y, orientation_pyr.r);
                        // the control state change processing loop will have written pitch/yaw to
                        // entity data, so now update direction
                        e.m_direction = fromCoHYpr(e.m_entity_data.m_orientation_pyr);
                    }

                    // todo(jbr) command to enable movement logging?
                    /*qCDebug(logMovement, "pos: (%1.8f, %1.8f, %1.8f)",
                            e.m_entity_data.m_pos.x,
                            e.m_entity_data.m_pos.y,
                            e.m_entity_data.m_pos.z);*/

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
                    /*if (controls->speed_scale_F0 != 0.0f)
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

                    // todo(jbr) command to enable movement logging?
                    //qCDebug(logMovement, "local_inpvel: (%1.8f, %1.8f, %1.8f)", local_input_velocity.x, local_input_velocity.y, local_input_velocity.z);

                    glm::vec3 input_velocity = e.m_direction * local_input_velocity;

                    e.m_motion_state.m_input_velocity = input_velocity;

                    // todo(jbr) command to enable movement logging?
                    //qCDebug(logMovement, "inpvel: (%1.8f, %1.8f, %1.8f)", input_velocity.x, input_velocity.y, input_velocity.z);
                    //qCDebug(logMovement, "pyr: (%1.8f, %1.8f, %1.8f)", e.m_entity_data.m_orientation_pyr.p, e.m_entity_data.m_orientation_pyr.y, e.m_entity_data.m_orientation_pyr.r);

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

                    // todo(jbr) command to enable movement logging?
                    /*qCDebug(logMovement, "move_time: %1.3f", e.m_motion_state.m_move_time);

                    qCDebug(logMovement, "newpos: (%1.8f, %1.8f, %1.8f)",
                            e.m_entity_data.m_pos.x,
                            e.m_entity_data.m_pos.y,
                            e.m_entity_data.m_pos.z);*/


                    // based on pmotionResetMoveTime()
                    for (int i = 0; i < 6; ++i)
                    {
                        // reset total time if button is released, or reverse button is pressed
                        if (!input_state->m_keys[i] || input_state->m_keys[s_reverse_control_dir[i]])
                        {
                            input_state->m_key_press_duration_ms[i] = 0;
                        }
                    }

                    // todo(jbr) command to enable movement logging?
                    //qCDebug(logMovement, "");

                    tick_state = {};

                break;
            }
        }
// todo(jbr) Q_ASSERTs
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
