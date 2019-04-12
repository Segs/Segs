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

void roundVelocityToZero(glm::vec3 *vel)
{
    if(std::abs(vel->x) < 0.00001f)
        vel->x = 0;
    if(std::abs(vel->y) < 0.00001f)
        vel->y = 0;
    if(std::abs(vel->z) < 0.00001f)
        vel->z = 0;
}

void processDirectionControl(InputState *next_state, uint8_t control_id, int ms_since_prev, int keypress_state)
{
    float delta = 0.0f;

    if(keypress_state)
        delta = 1.0f;

    qCDebug(logInput, "Pressed dir: %s \t ms_since_prev: %d \t press_release: %d", control_name[control_id], ms_since_prev, keypress_state);
    switch(control_id)
    {
        case 0: next_state->m_pos_delta[2] = delta; break;    // FORWARD
        case 1: next_state->m_pos_delta[2] = -delta; break;   // BACKWARD
        case 2: next_state->m_pos_delta[0] = -delta; break;   // LEFT
        case 3: next_state->m_pos_delta[0] = delta; break;    // RIGHT
        case 4: next_state->m_pos_delta[1] = delta; break;    // UP
        case 5: next_state->m_pos_delta[1] = -delta; break;   // DOWN
    }

    switch(control_id)
    {
        case 0:
        case 1: next_state->m_pos_delta_valid[2] = true; break;
        case 2:
        case 3: next_state->m_pos_delta_valid[0] = true; break;
        case 4:
        case 5: next_state->m_pos_delta_valid[1] = true; break;
    }
}

void setVelocity(Entity &e) // pmotionSetVel
{
    InputState  *state  = &e.m_states.m_inp_states.back();
    MotionState *motion = &e.m_motion_state;
    glm::vec3   horiz_vel       = {0, 0, 0};
    glm::vec3   vel             = {0, 0, 0};

    float       control_amounts[6] = {0};
    int         max_press_time  = 0;
    float       vel_scale_copy  = state->m_velocity_scale;

    if(state->m_no_collision)
        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;

    if(state->m_no_collision
            && e.m_player->m_options.alwaysmobile
            && (motion->m_controls_disabled || state->m_landing_recovery_time || state->m_controls_disabled))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Input Vel is <0, 0, 0>. No coll?" << state->m_no_collision;

        motion->m_input_velocity = vel;
    }
    else
    {
        for (int i = BinaryControl::FORWARD; i < BinaryControl::LAST_BINARY_VALUE; ++i)
        {
            float press_time = state->m_keypress_time[i];

            if(press_time > 0)
                qCDebug(logMovement) << "keypress_time" << i << press_time;

            if(press_time > max_press_time)
                max_press_time = state->m_keypress_time[i];

            if(i >= BinaryControl::UP && !motion->m_is_flying) // UP or Fly
                control_amounts[i] = (float)(press_time != 0);
            else if(!press_time)
                control_amounts[i] = 0.0f;
            else if(press_time >= 1000)
                control_amounts[i] = 1.0f;
            else if(press_time <= 50 && state->m_control_bits[i])
                control_amounts[i] = 0.0f;
            else if(press_time >= 75)
            {
                if(press_time < 75 || press_time >= 100)
                    control_amounts[i] = (press_time - 100) * 0.004f / 9.0f + 0.6f;
                else
                    control_amounts[i] = std::pow((press_time - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
            }
            else
                control_amounts[i] = 0.2f;

            if(control_amounts[i] > 0.0f)
                qCDebug(logMovement) << "control_amounts:" << i << control_amounts[i];
        }

        state->m_move_time = max_press_time;
        vel.x = control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT];
        vel.y = control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN];
        vel.z = control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD];
        vel.x = vel.x * motion->m_speed.x;
        vel.y = vel.y * motion->m_speed.y;
        horiz_vel = vel;

        if(e.m_type == EntType::PLAYER && glm::length(vel))
            qCDebug(logMovement) << "vel:" << glm::to_string(vel).c_str();

        if(!motion->m_is_flying)
            horiz_vel.y = 0.0f;

        if(vel.z < 0.0f)
            vel_scale_copy *= motion->m_backup_spd;

        if(motion->m_is_stunned)
            vel_scale_copy *= 0.1f;

        // Client returns 0 for negative length vector
        if(glm::length(horiz_vel) <= 0.0f)
            horiz_vel = glm::vec3(0,0,0);
        else
            horiz_vel = glm::normalize(horiz_vel);

        if(e.m_type == EntType::PLAYER && glm::length(horiz_vel))
            qCDebug(logMovement) << "horizVel:" << glm::to_string(horiz_vel).c_str();

        if(state->m_speed_scale != 0.0f)
            vel_scale_copy *= state->m_speed_scale;

        motion->m_velocity_scale = vel_scale_copy;
        vel.x = horiz_vel.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
        vel.z = horiz_vel.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);

        if(motion->m_is_flying)
            vel.y = horiz_vel.y * std::fabs(control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN]);
        else if(e.m_states.previous()->m_control_bits[BinaryControl::UP] && !state->m_control_bits[BinaryControl::UP])
            vel.y = 0.0f;
        else
        {
            vel.y *= glm::clamp(motion->m_jump_height, 0.0f, 1.0f);
            if(!e.m_motion_state.m_is_sliding)
                e.m_motion_state.m_flag_5 = false; // flag_5 moving on y-axis?
        }
     }

    // Movement Flags
    if(motion->m_is_flying)
        e.m_move_type |= MoveType::MOVETYPE_FLY;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_FLY;

    if(motion->m_has_jumppack)
        e.m_move_type |= MoveType::MOVETYPE_JETPACK;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_JETPACK;

    motion->m_input_velocity = vel;

    if(e.m_type == EntType::PLAYER && glm::length(vel))
        qCDebug(logMovement) << "final vel:" << glm::to_string(vel).c_str();

    if(e.m_char->m_char_data.m_afk && motion->m_input_velocity != glm::vec3(0,0,0))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Moving so turning off AFK";

        toggleAFK(*e.m_char);
    }

    // setPlayerVelQuat(&vel, vel_scale_copy); // we don't need this?
    motion->m_velocity = vel;
}

void processNewInputs(Entity &e)
{
    StateStorage* input_state = &e.m_states;
    for (InputState* new_input = input_state->m_inp_states.begin(); new_input != input_state->m_inp_states.end(); ++new_input)
    {
        for (ControlStateChange* csc = new_input->m_control_state_changes.begin();
             csc != new_input->m_control_state_changes.end();
             ++csc)
        {
            if (isControlStateChangeIdNewer(input_state->m_current_control_state_change_id, csc->first_id))
            {
                continue;
            }

            Q_ASSERT(input_state->m_current_control_state_change_id == csc->first_id);

            uint32_t key_press_start_ms[6] = {};
            for (const ControlStateChange::KeyChange& key_change : csc->key_changes)
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
                            if (input_state->m_key_press_time_ms[key_change.key] == 0)
                            {
                                input_state->m_key_press_time_ms[key_change.key] = 1;
                            }
                        }
                        else
                        {
                            input_state->m_key_press_time_ms[key_change.key] = std::min<uint32_t>(input_state->m_key_press_time_ms[key_change.key] + key_press_duration, 1000);
                        }
                    }
                }
            }

            // if any keys have been held for 250+ ms, then all keys
            // are given a minumum of 250ms press time
            uint32_t minimum_key_press_time = 0;
            for (int i = 0; i < 6; ++i)
            {
                if (input_state->m_key_press_time_ms[i] >= 250)
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
                input_state->m_key_press_time_ms[i] = std::max(
                    std::min<uint32_t>(
                        input_state->m_key_press_time_ms[i] + csc->tick_length_ms - key_press_start_ms[i],
                        1000),
                    minimum_key_press_time);
            }

            for (int i = 0; i < 6; ++i)
            {
                if (input_state->m_key_press_time_ms[i])
                {
                    qCDebug(logMovement, "%s%s (%dms)", input_state->m_keys[i] ? "+" : "-", s_key_name[i], input_state->m_key_press_time_ms[i]);
                }
            }

            float control_amounts[6] = {};
            uint32_t max_press_time = 0;
            for (int i = 0; i < 6; ++i)
            {
                uint32_t press_time = input_state->m_key_press_time_ms[i];
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

            glm::vec3 orientation_pyr = e.m_entity_data.m_orientation_pyr;
            qCDebug(logMovement, "current pyr = (%f, %f, %f)", orientation_pyr.p, orientation_pyr.y, orientation_pyr.r);
            bool orientation_changed = false;
            if (csc->pitch_changed)
            {
                qCDebug(logMovement, "pitch changed to %f", csc->pitch);
                orientation_pyr.x = csc->pitch;
                orientation_changed = true;
            }
            if (csc->yaw_changed)
            {
                qCDebug(logMovement, "yaw changed to %f", csc->yaw);
                orientation_pyr.y = csc->yaw;
                orientation_changed = true;
            }
            if (orientation_changed)
            {
                qCDebug(logMovement, "new pyr = (%f, %f, %f)", orientation_pyr.p, orientation_pyr.y, orientation_pyr.r);
                e.m_entity_data.m_orientation_pyr = orientation_pyr;
                e.m_direction = fromCoHYpr(orientation_pyr);
            }

            if (csc->no_collision_changed)
            {
                if (csc->no_collision)
                {
                    e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
                }
                else
                {
                    e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;
                }
            }

            qCDebug(logMovement, "pos: (%1.8f, %1.8f, %1.8f)",
                    e.m_entity_data.m_pos.x,
                    e.m_entity_data.m_pos.y,
                    e.m_entity_data.m_pos.z);

            glm::vec3 local_input_velocity(0.0f, 0.0f, 0.0f);
            local_input_velocity.x = control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT];
            //vel.y = control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN];
            local_input_velocity.y = 0.0f;
            local_input_velocity.z = control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD];
            //vel.x = vel.x * optrel_speeds->speed.x;
            //vel.y = vel.y * optrel_speeds->speed.y;
            glm::vec3 local_input_velocity_xz = local_input_velocity;
            local_input_velocity_xz.y = 0.0f;

            //if (vel.z < 0.0f)
            //    vel_scale_copy = vel_scale_copy * optrel_speeds->speed_back;
            //if (optrel_speeds->stunned)
            //    vel_scale_copy = vel_scale_copy * 0.1f;
            if (glm::length2(local_input_velocity_xz) > glm::epsilon<float>())
            {
                local_input_velocity_xz = glm::normalize(local_input_velocity_xz);
            }
            //if (controls->speed_scale_F0 != 0.0f)
            //    vel_scale_copy = vel_scale_copy * controls->speed_scale_F0;
            //controls->field_100 = vel_scale_copy;
            local_input_velocity.x = local_input_velocity_xz.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
            local_input_velocity.z = local_input_velocity_xz.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);

            qCDebug(logMovement, "local_inpvel: (%1.8f, %1.8f, %1.8f)", local_input_velocity.x, local_input_velocity.y, local_input_velocity.z);

            glm::vec3 input_velocity = e.m_direction * local_input_velocity;

            qCDebug(logMovement, "inpvel: (%1.8f, %1.8f, %1.8f)", input_velocity.x, input_velocity.y, input_velocity.z);

            qCDebug(logMovement, "pyr: (%1.8f, %1.8f, %1.8f)", e.m_entity_data.m_orientation_pyr.p, e.m_entity_data.m_orientation_pyr.y, e.m_entity_data.m_orientation_pyr.r);

            float timestep = 1.0f; // todo(jbr) can this change?
            e.m_motion_state.m_move_time += timestep;
            float time_scale = (e.m_motion_state.m_move_time + 6.0f) / 6.0f;
            time_scale = std::min(time_scale, 50.0f);

            e.m_entity_data.m_pos += time_scale * timestep * input_velocity;

            if (glm::length2(local_input_velocity) < glm::epsilon<float>())
            {
                e.m_motion_state.m_move_time = 0.0f;
            }

            qCDebug(logMovement, "move_time: %1.3f", e.m_motion_state.m_move_time);

            qCDebug(logMovement, "newpos: (%1.8f, %1.8f, %1.8f)",
                    e.m_entity_data.m_pos.x,
                    e.m_entity_data.m_pos.y,
                    e.m_entity_data.m_pos.z);

            input_state->m_current_control_state_change_id = csc->last_id + 1;

            for (int i = 0; i < 6; ++i)
            {
                // reset total time if button is released, or reverse button is pressed
                if (!input_state->m_keys[i] || input_state->m_keys[s_reverse_control_dir[i]])
                {
                    input_state->m_key_press_time_ms[i] = 0;
                }
            }

            qCDebug(logMovement, "");
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

bool updateRotation(const Entity &src, int axis ) /* returns true if given axis needs updating */
{
    if(src.m_states.previous() == nullptr)
        return true;

    if(src.m_states.previous()->m_orientation_pyr[axis] == src.m_states.current()->m_orientation_pyr[axis])
        return false;

    return true;
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
