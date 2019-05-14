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

namespace
{
const glm::mat3  s_identity_matrix = glm::mat3(1.0f);
//int              s_landed_on_ground = 0;
//static CollInfo     s_last_surf;
constexpr const int s_reverse_control_dir[6] = {
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

}

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

// state which resets every tick and is affected
// by processing control state changes
struct TickState
{
    uint32_t length_ms = 0;
    uint32_t key_press_start_ms[6] = {};
    bool    key_released[6] = {};
    bool    orientation_changed = false;
    uint8_t padding[1];
};

static void playerMotionUpdateControlsPrePhysics(Entity* player, const TickState* tick_state) // based on pmotionUpdateControlsPrePhysics
{
    if (player->m_motion_state.m_controls_disabled && !player->m_player->m_options.alwaysmobile)
    {
        for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
        {
            player->m_input_state.m_key_press_duration_ms[key] = 0;
        }
        return;
    }

    // if any keys have been held for 250+ ms, then all keys
    // are given a minumum of 250ms press time
    uint32_t minimum_key_press_time = 0;
    if (player->m_motion_state.m_is_jumping)
    {
        minimum_key_press_time = 250;
    }
    else
    {
        for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
        {
            if (player->m_input_state.m_key_press_duration_ms[key] >= 250)
            {
                minimum_key_press_time = 250;
                break;
            }
        }
    }

    for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key )
    {
        if (!player->m_input_state.m_keys[key] || player->m_input_state.m_keys[s_reverse_control_dir[key]]) // not pressed or pressed both direction at once
        {
            continue;
        }

        // update key press time, keeping within min/max range
        const uint32_t current_duration = player->m_input_state.m_key_press_duration_ms[key];
        uint32_t milliseconds = tick_state->length_ms - tick_state->key_press_start_ms[key];

        player->m_input_state.m_key_press_duration_ms[key] = glm::clamp<uint32_t>(
                    current_duration + milliseconds,
                    minimum_key_press_time,
                    1000);

        if (logMovement().isDebugEnabled() && player->m_motion_state.m_debug)
        {
            qCDebug(logMovement, "\nAdding: %s += \t%dms (%dms total)\n",
                    s_key_name[key],
                    milliseconds,
                    player->m_input_state.m_key_press_duration_ms[key]);
        }
    }
}

static void playerMotionSetInputVelocity(Entity* player, const TickState* tick_state) // based on pmotionSetVel()
{
    if (player->m_motion_state.m_no_collision)
    {
        player->m_move_type |= MOVETYPE_NOCOLL;
    }
    else
    {
        player->m_move_type &= ~MOVETYPE_NOCOLL;
    }

    glm::vec3 local_input_velocity(0.0f, 0.0f, 0.0f);

    if (!player->m_motion_state.m_no_collision
            && !player->m_player->m_options.alwaysmobile
            && (player->m_motion_state.m_controls_disabled
             || player->m_motion_state.m_has_headpain
             //|| controls->controls_disabled not sure why client appears to check this twice?
             ))
    {
        local_input_velocity = { 0,0,0 };
    }
    else
    {
        float control_amounts[6] = {};
        for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
        {
            uint32_t press_time = player->m_input_state.m_key_press_duration_ms[key];

            if (key >= BinaryControl::UP && !player->m_motion_state.m_is_flying)
            {
                control_amounts[key] = (float)(press_time != 0);
            }
            else if (!press_time )
            {
                control_amounts[key] = 0.0f;
            }
            else if (press_time >= 1000)
            {
                control_amounts[key] = 1.0f;
            }
            else if (press_time <= 50 && player->m_input_state.m_keys[key])
            {
                control_amounts[key] = 0.0;
            }
            else if (press_time >= 75)
            {
                if (press_time < 75 || press_time >= 100)
                {
                    control_amounts[key] = (float)(press_time - 100) * 0.004f / 9.0f + 0.6f;
                }
                else
                {
                    control_amounts[key] = std::pow((float)(press_time - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
                }
            }
            else
            {
                control_amounts[key] = 0.2f;
            }
        }

        local_input_velocity.x = control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT];
        local_input_velocity.y = control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN];
        local_input_velocity.z = control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD];
        local_input_velocity.x *= player->m_motion_state.m_speed.x;
        local_input_velocity.y *= player->m_motion_state.m_speed.y;

        glm::vec3 local_input_velocity_xz = local_input_velocity;
        if (!player->m_motion_state.m_is_flying)
        {
            local_input_velocity_xz.y = 0;
        }

        float input_velocity_scale = player->m_input_state.m_velocity_scale;
        if (local_input_velocity.z < 0.0f)
        {
            input_velocity_scale = input_velocity_scale * player->m_motion_state.m_backup_spd;
        }
        if (player->m_motion_state.m_is_stunned)
        {
            input_velocity_scale = input_velocity_scale * 0.1f;
        }
        if (player->m_player->m_options.speed_scale != 0.0f)
        {
            input_velocity_scale *= player->m_player->m_options.speed_scale;
        }
        player->m_motion_state.m_velocity_scale = input_velocity_scale;

        if (glm::length2(local_input_velocity_xz) > std::numeric_limits<float>::epsilon())
        {
            local_input_velocity_xz = glm::normalize(local_input_velocity_xz);
        }

        local_input_velocity.x = local_input_velocity_xz.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
        local_input_velocity.z = local_input_velocity_xz.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);
        if (player->m_motion_state.m_is_flying)
        {
            local_input_velocity.y = local_input_velocity_xz.y * std::fabs(control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN]);
        }
        else if (tick_state->key_released[BinaryControl::UP])
        {
            local_input_velocity.y = 0;
        }
        else
        {
            local_input_velocity.y *= glm::clamp<float>(player->m_motion_state.m_jump_height, 0.0f, 1.0f);

            if (!player->m_motion_state.m_is_sliding)
            {
                player->m_motion_state.m_flag_5 = false;
            }
        }
    }

    if (player->m_motion_state.m_is_flying)
    {
        player->m_move_type |= MOVETYPE_FLY;
    }
    else
    {
        player->m_move_type &= ~MOVETYPE_FLY;
    }

    if (player->m_motion_state.m_has_jumppack)
    {
        player->m_move_type |= MOVETYPE_JETPACK;
    }
    else
    {
        player->m_move_type &= ~MOVETYPE_JETPACK;
    }

    player->m_motion_state.m_input_velocity = local_input_velocity;

    if (player->m_char->m_char_data.m_afk &&
        glm::length2(player->m_motion_state.m_input_velocity) > std::numeric_limits<float>::epsilon())
    {
        if(logMovement().isDebugEnabled() && player->m_motion_state.m_debug && player->m_type == EntType::PLAYER)
        {
            qCDebug(logMovement) << "Moving so turning off AFK";
        }

        setAFK(*player->m_char, false);
    }
}

// based on entMoveNoCollision
static void playerMoveNoCollision(Entity* player, float timestep)
{
    player->m_motion_state.m_is_falling = true;
    player->m_motion_state.m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    player->m_motion_state.m_move_time += timestep;
    float time_scale = (player->m_motion_state.m_move_time + 6.0f) / 6.0f;
    time_scale = std::min(time_scale, 50.0f);

    player->m_entity_data.m_pos += time_scale * timestep * player->m_motion_state.m_input_velocity;

    if (glm::length2(player->m_motion_state.m_input_velocity) <= std::numeric_limits<float>::epsilon())
    {
        player->m_motion_state.m_move_time = 0.0f;
    }
}

static void playerWalk(Entity* /*player*/)
{
    // not implemented
}

static bool checkPlayerCollision(Entity* /*player*/, bool /*bouncing*/)
{
    // not implemented
    return true;
}

// based on pmotionWithPrediction()
static void playerMotion(Entity* player, glm::vec3* final_input_velocity)
{
    float timestep = 1.0f;
    player->m_motion_state.m_input_velocity = *final_input_velocity;

    // will be needed later
    /*
    if (player->m_motion_state.m_landing_recovery_time)
    {
        --player->m_motion_state.m_landing_recovery_time;
    }*/

    // based on entMotion()
    if ( player->m_motion_state.m_is_falling
         || glm::length2(player->m_motion_state.m_velocity) > std::numeric_limits<float>::epsilon()
         || glm::length2(player->m_motion_state.m_input_velocity) > std::numeric_limits<float>::epsilon()
         || !(player->m_move_type & MOVETYPE_WALK)
         || (player->m_type == EntType::PLAYER && checkPlayerCollision(player, /*bouncing*/ false)) )
    {
        if (player->m_move_type & MOVETYPE_NOCOLL)
        {
            playerMoveNoCollision(player, timestep);
        }
        else if (player->m_move_type & MOVETYPE_WALK)
        {
            player->m_motion_state.m_move_time = 0;
            playerWalk(player);
            player->m_motion_state.m_input_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            player->m_motion_state.m_is_falling = false;
        }
    }
}

void processNewInputs(Entity &e)
{
    InputState* input_state = &e.m_input_state;
    for (auto input_change_iter = input_state->m_queued_changes.begin();
         input_change_iter != input_state->m_queued_changes.end();
         ++input_change_iter) // todo: buffer these for jitter/cheating
    {
        const InputStateChange& input_change = *input_change_iter;

        if (input_change.m_control_state_changes.size())
        {
            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
            {
                qCDebug(logInput, "csc range %hu->%hu",
                        input_change.m_first_control_state_change_id,
                        input_change.m_first_control_state_change_id + (uint16_t)input_change.m_control_state_changes.size() - 1);
            }

            // The client will re-send the same control state changes until acked, so
            // need to make sure we skip any changes we've seen before.
            int32_t csc_id_delta = cscIdDelta(input_change.m_first_control_state_change_id, input_state->m_next_expected_control_state_change_id);

            // if this is negative, it means there is a gap in control state changes, shouldn't ever happen
            assert(csc_id_delta >= 0);

            if (logInput().isDebugEnabled() && e.m_input_state.m_debug && csc_id_delta > 0)
            {
                qCDebug(logInput, "skipping %d changes", csc_id_delta);
            }

            // if csc_id_delta >= control_state_changes.size() then all of 
            // these changes are old and should be ignored
            if (csc_id_delta < (int32_t)input_change.m_control_state_changes.size())
            {
                TickState tick_state = {};

                // iterate control state changes
                for (auto csc_iter = input_change.m_control_state_changes.begin() + csc_id_delta;
                    csc_iter != input_change.m_control_state_changes.end();
                    ++csc_iter)
                {
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
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "key %hhu = %hhu", csc.control_id, csc.data.key_state);
                            }

                            uint8_t key = csc.control_id;

                            input_state->m_keys[key] = csc.data.key_state;

                            if (csc.data.key_state)
                            {
                                tick_state.key_press_start_ms[key] = tick_state.length_ms;
                            }
                            else
                            {
                                tick_state.key_released[key] = true;

                                if (!input_state->m_keys[s_reverse_control_dir[key]])
                                {
                                    uint32_t key_press_duration = tick_state.length_ms; // note: I have never observed a key press & release in the same tick, appears to be impossible
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
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "pitch=%f", csc.data.angle);
                            }

                            tick_state.orientation_changed = true;
                            e.m_entity_data.m_orientation_pyr.x = csc.data.angle;
                        break;

                        case BinaryControl::YAW:
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "yaw=%f", csc.data.angle);
                            }

                            tick_state.orientation_changed = true;
                            e.m_entity_data.m_orientation_pyr.y = csc.data.angle;
                        break;

                        case 9:
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "every_4_ticks=%hhu", csc.data.every_4_ticks);
                            }

                            input_state->m_every_4_ticks = csc.data.every_4_ticks;
                        break;

                        case 10:
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "nocoll=%d", csc.data.no_collision);
                            }

                            e.m_motion_state.m_no_collision = csc.data.no_collision;
                        break;

                        case 8:
                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                            {
                                qCDebug(logInput, "doing tick, controls_disabled=%d, time_diff_1=%u, time_diff_2=%u, velocity_scale=%hhu",
                                        csc.data.control_id_8.controls_disabled,
                                        csc.data.control_id_8.time_diff_1,
                                        csc.data.control_id_8.time_diff_2,
                                        csc.data.control_id_8.velocity_scale);
                            }

                            // do a tick!

                            // Save current position to last_pos
                            e.m_motion_state.m_last_pos = e.m_entity_data.m_pos;

                            input_state->m_velocity_scale = csc.data.control_id_8.velocity_scale / 255.0f;

                            e.m_motion_state.m_controls_disabled = csc.data.control_id_8.controls_disabled;

                            // following code is based on playerPredictMotion_betaname() in client
                            playerMotionUpdateControlsPrePhysics(&e, &tick_state);
                            playerMotionSetInputVelocity(&e, &tick_state);

                            if (tick_state.orientation_changed)
                            {
                                // the control state change processing loop will have written pitch/yaw to
                                // entity data, so now update direction
                                e.m_direction = fromCoHYpr(e.m_entity_data.m_orientation_pyr);
                            }

                            glm::vec3 final_input_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                            if (!e.m_motion_state.m_controls_disabled)
                            {
                                final_input_velocity = e.m_direction * e.m_motion_state.m_input_velocity;
                            }

                            playerMotion(&e, &final_input_velocity);

                            // based on reportPhysicsSteps()
                            if (logMovement().isDebugEnabled() && e.m_motion_state.m_debug)
                            {
                                static int32_t count = -1;

                                bool any_keys_relevant = false;
                                for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
                                {
                                    if (input_state->m_key_press_duration_ms[key])
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
                                    for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
                                    {
                                        if (input_state->m_key_press_duration_ms[key])
                                        {
                                            keys += QString::asprintf("%s%s (%dms), ",
                                                        tick_state.key_released[key] ? "-" : "+",
                                                        s_key_name[key],
                                                        input_state->m_key_press_duration_ms[key]);
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
                                            keys.toUtf8().constData(),
                                            e.m_motion_state.m_last_pos.x, e.m_motion_state.m_last_pos.y, e.m_motion_state.m_last_pos.z,
                                            e.m_motion_state.m_velocity.x, e.m_motion_state.m_velocity.y, e.m_motion_state.m_velocity.z,
                                            final_input_velocity.x, final_input_velocity.y, final_input_velocity.z,
                                            e.m_motion_state.m_velocity_scale,
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
                            for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
                            {
                                // reset total time if button is released, or reverse button is pressed
                                if (!input_state->m_keys[key] || input_state->m_keys[s_reverse_control_dir[key]])
                                {
                                    input_state->m_key_press_duration_ms[key] = 0;
                                }
                            }

                            tick_state = {};

                            if (logInput().isDebugEnabled() && e.m_input_state.m_debug && input_change.m_has_pitch_and_yaw)
                            {
                                qCDebug(logInput, "extended pitch=%f, yaw=%f", input_change.m_pitch, input_change.m_yaw);
                            }

                        break;
                    }
                }

                // assuming that the client never sends a partial tick, if that's not the case 
                // then tick_state needs to move into the entity somewhere
                assert(tick_state.length_ms == 0);

                uint16_t new_csc_count = input_change.m_control_state_changes.size() - csc_id_delta;

                if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                {
                    qCDebug(logInput, "processed csc %hu->%hu",
                        input_state->m_next_expected_control_state_change_id,
                        input_state->m_next_expected_control_state_change_id + new_csc_count - 1);
                }

                input_state->m_next_expected_control_state_change_id += new_csc_count;
            }
        }

        if (input_change.m_has_keys)
        {
            for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
            {
                if (input_change.m_keys[key] != input_state->m_keys[key])
                {
                    if (logInput().isDebugEnabled() && e.m_input_state.m_debug)
                    {
                        qCDebug(logInput, "keys input state mismatch");
                    }
                    input_state->m_keys[key] = input_change.m_keys[key];
                }
            }
        }

        if (logInput().isDebugEnabled() && e.m_input_state.m_debug && input_change.m_has_pitch_and_yaw)
        {
            qCDebug(logInput, "extended pitch=%f, yaw=%f", input_change.m_pitch, input_change.m_yaw);
        }
    }

    input_state->m_queued_changes.clear();
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
