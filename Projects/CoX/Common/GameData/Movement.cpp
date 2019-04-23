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
    StateStorage* input_state = &e.m_states;
    for (InputState* new_input = input_state->m_inp_states.begin(); new_input != input_state->m_inp_states.end(); ++new_input) // todo(jbr) buffer these for jitter/cheating
    {
        for (auto iter = new_input->m_control_state_changes.begin();
             iter != new_input->m_control_state_changes.end();
             ++iter)
        {
            ControlStateChangesForTick& changes_for_tick = *iter;

            if (isControlStateChangeIdNewer(input_state->m_next_expected_control_state_change_id, changes_for_tick.first_change_id)) // todo(jbr) surely just check this when adding them?
            {
                continue;
            }

            Q_ASSERT(input_state->m_next_expected_control_state_change_id == changes_for_tick.first_change_id);

            uint32_t key_press_start_ms[6] = {};
            for (const ControlStateChangesForTick::KeyChange& key_change : changes_for_tick.key_changes)
            {
                input_state->m_keys[key_change.key] = key_change.state;

                if (key_change.state)
                {
                    key_press_start_ms[key_change.key] = key_change.offset_from_tick_start_ms;
                }
                else
                {
                    if (!input_state->m_keys[s_reverse_control_dir[key_change.key]])
                    {
                        uint32_t key_press_duration = key_change.offset_from_tick_start_ms;
                        if (key_press_duration == 0)
                        {
                            if (input_state->m_key_press_duration_ms[key_change.key] == 0)
                            {
                                input_state->m_key_press_duration_ms[key_change.key] = 1;
                            }
                        }
                        else
                        {
                            input_state->m_key_press_duration_ms[key_change.key] = std::min<uint32_t>(input_state->m_key_press_duration_ms[key_change.key] + key_press_duration, 1000);
                        }
                    }
                }
            }

            // if any keys have been held for 250+ ms, then all keys
            // are given a minumum of 250ms press time
            uint32_t minimum_key_press_time = 0;
            for (int i = 0; i < 6; ++i)
            {
                if (input_state->m_key_press_duration_ms[i] >= 250)
                {
                    minimum_key_press_time = 250;
                    break;
                }
            }

            for (int i = 0; i < 6; ++i)
            {
                if (!input_state->m_keys[i] || input_state->m_keys[s_reverse_control_dir[i]])
                {
                    continue;
                }

                // update key press time, keeping within min/max range
                input_state->m_key_press_duration_ms[i] = glm::clamp<uint32_t>(input_state->m_key_press_duration_ms[i] + changes_for_tick.tick_length_ms - key_press_start_ms[i],
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

            float control_amounts[6] = {};
            uint32_t max_press_time = 0;
            for (int i = 0; i < 6; ++i)
            {
                uint32_t press_time = input_state->m_key_press_duration_ms[i];
                max_press_time = std::max(max_press_time, press_time);

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

            // todo(jbr) are pitch/yaw changes definitely tick start not tick end?
            glm::vec3 orientation_pyr = e.m_entity_data.m_orientation_pyr;
            bool orientation_changed = false;
            if (changes_for_tick.pitch_changed)
            {
                orientation_pyr.x = changes_for_tick.pitch;
                orientation_changed = true;
            }
            if (changes_for_tick.yaw_changed)
            {
                orientation_pyr.y = changes_for_tick.yaw;
                orientation_changed = true;
            }
            if (orientation_changed)
            {
                // todo(jbr) command to enable movement logging?
                //qCDebug(logMovement, "new pyr = (%f, %f, %f)", orientation_pyr.p, orientation_pyr.y, orientation_pyr.r);
                e.m_entity_data.m_orientation_pyr = orientation_pyr;
                e.m_direction = fromCoHYpr(orientation_pyr);
            }

            if (changes_for_tick.no_collision_changed)
            {
                // todo(jbr) command to enable movement logging?
                if (changes_for_tick.no_collision)
                {
                    e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
                }
                else
                {
                    e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;
                }
            }

            // todo(jbr) command to enable movement logging?
            /*qCDebug(logMovement, "pos: (%1.8f, %1.8f, %1.8f)",
                    e.m_entity_data.m_pos.x,
                    e.m_entity_data.m_pos.y,
                    e.m_entity_data.m_pos.z);*/

            glm::vec3 local_input_velocity(0.0f, 0.0f, 0.0f);
            local_input_velocity.x = control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT];
            local_input_velocity.y = control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN];
            local_input_velocity.y = 0.0f;
            local_input_velocity.z = control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD];
            local_input_velocity.x = local_input_velocity.x * e.m_motion_state.m_speed.x;
            local_input_velocity.y = local_input_velocity.y * e.m_motion_state.m_speed.y;

            glm::vec3 local_input_velocity_xz = local_input_velocity;
            if (!e.m_motion_state.m_is_flying)
            {
                local_input_velocity_xz.y = 0;
            }

            // todo(jbr) think this is client only, do we still want it anyway?
            /*float input_velocity_scale = cs->inp_vel_scale;

            if (local_input_velocity_xz.z < 0.0f)
            {
                input_velocity_scale = input_velocity_scale * optrel_speeds->speed_back;
            }
            if (optrel_speeds->stunned)
            {
                input_velocity_scale = input_velocity_scale * 0.1f;
            }
            if (controls->speed_scale_F0 != 0.0f)
            {
                input_velocity_scale = input_velocity_scale * controls->speed_scale_F0;
            }
            e.m_motion_state.m_velocity_scale = input_velocity_scale; */

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
            else if (false) //controls->bs_38_1[BinaryControl::UP]) todo(jbr)
            {
                local_input_velocity.y = 0;
            }
            else
            {
                local_input_velocity.y *= glm::clamp<float>(e.m_motion_state.m_jump_height, 0.0f, 1.0f);
                // todo(jbr)
                /*if (!optrel_speeds->flags_178_20)
                {
                    ent->motion.flag_5 = false;
                }*/
            }

            // todo(jbr) command to enable movement logging?
            //qCDebug(logMovement, "local_inpvel: (%1.8f, %1.8f, %1.8f)", local_input_velocity.x, local_input_velocity.y, local_input_velocity.z);

            glm::vec3 input_velocity = e.m_direction * local_input_velocity;

            // todo(jbr) command to enable movement logging?
            //qCDebug(logMovement, "inpvel: (%1.8f, %1.8f, %1.8f)", input_velocity.x, input_velocity.y, input_velocity.z);
            //qCDebug(logMovement, "pyr: (%1.8f, %1.8f, %1.8f)", e.m_entity_data.m_orientation_pyr.p, e.m_entity_data.m_orientation_pyr.y, e.m_entity_data.m_orientation_pyr.r);

            // todo(jbr) check move type is nocoll here, put unimplemented block for all else
            float timestep = 1.0f; // todo(jbr) can this change?
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

            input_state->m_next_expected_control_state_change_id = changes_for_tick.last_change_id + 1;

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
        }

        // todo(jbr) check the keypress state at end of packet, make sure all matches up
    }

    input_state->m_inp_states.clear();
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
