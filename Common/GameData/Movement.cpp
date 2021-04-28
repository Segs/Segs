/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Movement.h"

#include "Entity.h"
#include "EntityHelpers.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Common/GameData/CoHMath.h"
#include "Common/GameData/seq_definitions.h"
#include "Common/GameData/playerdata_definitions.h"
#include "Components/Logging.h"

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

static void roundVelocityToZero(glm::vec3 *vel)
{
    if (std::abs(vel->x) < 0.00001f)
        vel->x = 0;
    if (std::abs(vel->y) < 0.00001f)
        vel->y = 0;
    if (std::abs(vel->z) < 0.00001f)
        vel->z = 0;
}

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

// movement state which resets every tick and is affected
// by processing control state changes
struct TickState
{
    uint32_t length_ms = 0;
    uint32_t key_press_start_ms[6] = {};
    bool    key_released[6] = {};
    bool    orientation_changed = false;
    uint8_t padding[1];
};

static void entWorldGetSurface(Entity *ent, SurfaceParams *surf_params)
{
    if (ent->m_motion_state.m_is_falling || ent->m_motion_state.m_is_flying)
        *surf_params = g_world_surf_params[1];
    else
        *surf_params = g_world_surf_params[0];

    if(!(ent->m_motion_state.m_is_falling || ent->m_motion_state.m_is_flying))
    {
        float traction = 1.0f - ent->m_motion_state.m_surf_normal.y;
        if ( !ent->m_motion_state.m_flag_13 && traction >= 0.3f ) //makeSteepSlopesSlippery
        {
            float friction = (traction - 0.3f) / 0.3f;
            if ( friction > 1.0f )
                friction = 1.0f;

            if ( !ent->m_motion_state.m_flag_12 )
                surf_params->traction = (1.0f - friction) * surf_params->traction + friction * 0.001f;

            surf_params->friction = (1.0f - friction) * surf_params->friction + friction * 0.001f;
        }
    }
}

static SurfaceParams *getSurfaceModifier(Entity *e)
{
    static SurfaceParams surf;
    bool is_in_air = (e->m_motion_state.m_is_falling || e->m_motion_state.m_is_flying) != 0;
    surf = e->m_motion_state.m_surf_mods[is_in_air];

    if (e->m_motion_state.m_is_flying || e->m_motion_state.m_is_jumping)
    {
        surf.gravitational_constant = 0;
        return &surf;
    }

    if (is_in_air)
    {
        surf.friction = 1.0f;
        if ( !e->m_motion_state.m_is_falling )
            surf.traction = 1.0f;
    }

    return &surf;
}

static void entWorldApplySurfMods(SurfaceParams *surf1, SurfaceParams *surf2)
{
    surf2->traction = surf1->traction * surf2->traction;
    surf2->friction = surf1->friction * surf2->friction;
    surf2->bounce   = surf1->bounce * surf2->bounce;
    surf2->gravitational_constant  = surf1->gravitational_constant * surf2->gravitational_constant;
    surf2->max_speed = surf1->max_speed * surf2->max_speed;
}

static void applySurfModsByFlags(SurfaceParams *surf, int walk_flags)
{
    if ( !walk_flags || !surf )
        return;

    if ( walk_flags & CollFlags::COLL_SURF_SLICK )
    {
        surf->friction = 0.01f;
        surf->traction = 0.2f;
    }

    if ( walk_flags & CollFlags::COLL_SURF_BOUNCY )
        surf->bounce = 0.5f;

    if ( walk_flags & CollFlags::COLL_SURF_ICY )
    {
        surf->friction = 0;
        surf->traction = 0.05f;
    }
}

static void checkJump(Entity *ent, SurfaceParams *surf_params)
{
    bool want_jump = false;

    if(ent->m_motion_state.m_jump_time > 0)
        --ent->m_motion_state.m_jump_time;

    if(ent->m_move_type & MoveType::MOVETYPE_FLY)
        return;

    if(ent->m_motion_state.m_input_velocity.y > 0.001f) // formerly: input_vel
    {
        if ((ent->m_motion_state.m_is_stunned)
                || (!ent->m_motion_state.m_has_jumppack && ent->m_motion_state.m_surf_normal.y <= 0.3f))
        {
            ent->m_motion_state.m_input_velocity.y = 0; // formerly: input_vel
        }
        else
            want_jump = true;
    }

    if (ent->m_motion_state.m_velocity.y > 0.0f && ent->m_motion_state.m_has_jumppack
         && (!want_jump && !(ent->m_move_type & MoveType::MOVETYPE_JETPACK)))
    {
        ent->m_motion_state.m_has_jumppack = false;
        ent->m_motion_state.m_velocity.y -= std::min(1.5f, ent->m_motion_state.m_velocity.y) * 0.5f;
    }

    bool not_moving_on_yaxis = !(ent->m_motion_state.m_is_falling
                                 || ent->m_motion_state.m_is_flying
                                 || ent->m_motion_state.m_jump_time > 0
                                 || ent->m_move_type & MoveType::MOVETYPE_JETPACK);

    if(!ent->m_motion_state.m_is_jumping)
    {
        if(want_jump)
        {
            if(!not_moving_on_yaxis || ent->m_motion_state.m_flag_5)
                ent->m_motion_state.m_input_velocity.y = 0;
            else
            {
                ent->m_motion_state.m_is_jumping        = true;
                ent->m_motion_state.m_is_bouncing       = true;
                ent->m_motion_state.m_jump_start        = ent->m_entity_data.m_pos.y;
                ent->m_motion_state.m_max_jump_height   = ent->m_motion_state.m_jump_height * 4.0f;
                ent->m_motion_state.m_jump_time         = 15;
                ent->m_motion_state.m_has_jumppack      = true;
                ent->m_motion_state.m_flag_5            = true;
            }
        }
    }
    else
    {
        if (!(ent->m_move_type & MoveType::MOVETYPE_JETPACK)
                && ent->m_motion_state.m_stuck_head
                || ent->m_entity_data.m_pos.y - ent->m_motion_state.m_jump_start >= ent->m_motion_state.m_max_jump_height
                || !want_jump
                || ent->m_motion_state.m_is_falling)
        {
            ent->m_motion_state.m_is_jumping =  false;
        }
    }

    if (ent->m_move_type & MoveType::MOVETYPE_JETPACK)
    {
        if (want_jump)
        {
            ent->m_motion_state.m_is_falling    = false;
            ent->m_motion_state.m_flag_1        = false;
            ent->m_motion_state.m_is_jumping    = true;
        }
        else
            ent->m_motion_state.m_is_jumping    = false;

        // TODO: Flesh out Seq system
        ent->m_seq_state.setVal(SeqBitNames::JETPACK, ent->m_motion_state.m_is_jumping);
    }
    else if (ent->m_motion_state.m_input_velocity.y != 0.0f
             && surf_params->gravitational_constant != 0.0f
             && ent->m_motion_state.m_is_falling)
    {
        ent->m_motion_state.m_input_velocity.y = 0;
    }
}

static void doPhysicsOnce(Entity *ent, SurfaceParams *surf_params)
{
    ent->m_motion_state.m_surf_repulsion = glm::vec3(0,0,0);
    //entWorldCollide(ent, surf_params); TODO
    ent->m_motion_state.m_velocity += ent->m_motion_state.m_surf_repulsion;
}

static void doPhysics(Entity *ent, SurfaceParams *surf_mods)
{
    float timestep = ent->m_input_state.m_timestep;
    float mag = glm::length(ent->m_motion_state.m_velocity) * timestep;

    if (mag >= 1.0f || timestep >= 5.0f)
    {
        int numsteps = int32_t((mag + 0.99f) / 1.0f);

        if (numsteps > 5 || timestep >= 5.0f)
            numsteps = 5;

        ent->m_input_state.m_timestep /= float(numsteps);

        for(int i=0; i<numsteps; ++i)
            doPhysicsOnce(ent, surf_mods);

        ent->m_input_state.m_timestep *= numsteps;
    }
    else
        doPhysicsOnce(ent, surf_mods);

    //if(!ent->seq) // presumably some entities wont have sequences
        //return;

    if(ent->m_motion_state.m_is_falling)
    {
        if (ent->m_motion_state.m_velocity.y > 0.0f && ent->m_motion_state.m_velocity.y < 0.3f)
            ent->m_seq_state.set(SeqBitNames::APEX);

        if(ent->m_motion_state.m_velocity.y < 0.0f)
        {
            if(!(ent->m_move_type & MoveType::MOVETYPE_JETPACK))
                ent->m_motion_state.m_is_jumping = false; // no longer counted as jump, we're falling

            ent->m_seq_state.setVal(SeqBitNames::JUMP, false);
        }

        if(ent->m_motion_state.m_is_falling)
            ent->m_seq_state.set(SeqBitNames::AIR);
    }
    if(ent->m_motion_state.m_is_jumping)
    {
        if(ent->m_move_type & MoveType::MOVETYPE_JETPACK)
            ent->m_seq_state.set(SeqBitNames::JETPACK);
    }
}

static void entMotionUpdateControlsPrePhysics(Entity* ent, const TickState* tick_state) // based on pmotionUpdateControlsPrePhysics
{
    if (ent->m_motion_state.m_controls_disabled && !ent->m_player->m_options.alwaysmobile)
    {
        for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
        {
            ent->m_input_state.m_key_press_duration_ms[key] = 0;
        }
        return;
    }

    // if any keys have been held for 250+ ms, then all keys
    // are given a minumum of 250ms press time
    uint32_t minimum_key_press_time = 0;
    if (ent->m_motion_state.m_is_jumping)
    {
        minimum_key_press_time = 250;
    }
    else
    {
        for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key)
        {
            if (ent->m_input_state.m_key_press_duration_ms[key] >= 250)
            {
                minimum_key_press_time = 250;
                break;
            }
        }
    }

    for (int key = 0; key <= BinaryControl::LAST_BINARY_VALUE; ++key )
    {
        if (!ent->m_input_state.m_keys[key] || ent->m_input_state.m_keys[s_reverse_control_dir[key]]) // not pressed or pressed both direction at once
        {
            continue;
        }

        // update key press time, keeping within min/max range
        const uint32_t current_duration = ent->m_input_state.m_key_press_duration_ms[key];
        uint32_t milliseconds = tick_state->length_ms - tick_state->key_press_start_ms[key];

        ent->m_input_state.m_key_press_duration_ms[key] = glm::clamp<uint32_t>(
                    current_duration + milliseconds,
                    minimum_key_press_time,
                    1000);

        if (logMovement().isDebugEnabled() && ent->m_motion_state.m_debug)
        {
            qCDebug(logMovement, "\nAdding: %s += \t%dms (%dms total)\n",
                    s_key_name[key],
                    milliseconds,
                    ent->m_input_state.m_key_press_duration_ms[key]);
        }
    }
}

static void entMotionSetInputVelocity(Entity* ent, const TickState* tick_state) // based on pmotionSetVel()
{
    if (ent->m_motion_state.m_no_collision)
    {
        ent->m_move_type |= MOVETYPE_NOCOLL;
    }
    else
    {
        ent->m_move_type &= ~MOVETYPE_NOCOLL;
    }

    glm::vec3 local_input_velocity(0.0f, 0.0f, 0.0f);

    if (!ent->m_motion_state.m_no_collision
            && !ent->m_player->m_options.alwaysmobile
            && (ent->m_motion_state.m_controls_disabled
             || ent->m_motion_state.m_has_headpain
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
            uint32_t press_time = ent->m_input_state.m_key_press_duration_ms[key];

            if (key >= BinaryControl::UP && !ent->m_motion_state.m_is_flying)
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
            else if (press_time <= 50 && ent->m_input_state.m_keys[key])
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
        local_input_velocity.x *= ent->m_motion_state.m_speed.x;
        local_input_velocity.y *= ent->m_motion_state.m_speed.y;

        glm::vec3 local_input_velocity_xz = local_input_velocity;
        if (!ent->m_motion_state.m_is_flying)
        {
            local_input_velocity_xz.y = 0;
        }

        float input_velocity_scale = ent->m_input_state.m_velocity_scale;
        if (local_input_velocity.z < 0.0f)
        {
            input_velocity_scale = input_velocity_scale * ent->m_motion_state.m_backup_spd;
        }
        if (ent->m_motion_state.m_is_stunned)
        {
            input_velocity_scale = input_velocity_scale * 0.1f;
        }
        if (ent->m_player->m_options.speed_scale != 0.0f)
        {
            input_velocity_scale *= ent->m_player->m_options.speed_scale;
        }
        ent->m_motion_state.m_velocity_scale = input_velocity_scale;

        if (glm::length2(local_input_velocity_xz) > std::numeric_limits<float>::epsilon())
        {
            local_input_velocity_xz = glm::normalize(local_input_velocity_xz);
        }

        local_input_velocity.x = local_input_velocity_xz.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
        local_input_velocity.z = local_input_velocity_xz.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);
        if (ent->m_motion_state.m_is_flying)
        {
            local_input_velocity.y = local_input_velocity_xz.y * std::fabs(control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN]);
        }
        else if (tick_state->key_released[BinaryControl::UP])
        {
            local_input_velocity.y = 0;
        }
        else
        {
            local_input_velocity.y *= glm::clamp<float>(ent->m_motion_state.m_jump_height, 0.0f, 1.0f);

            if (!ent->m_motion_state.m_is_sliding)
            {
                ent->m_motion_state.m_flag_5 = false;
            }
        }
    }

    if (ent->m_motion_state.m_is_flying)
    {
        ent->m_move_type |= MOVETYPE_FLY;
    }
    else
    {
        ent->m_move_type &= ~MOVETYPE_FLY;
    }

    if (ent->m_motion_state.m_has_jumppack)
    {
        ent->m_move_type |= MOVETYPE_JETPACK;
    }
    else
    {
        ent->m_move_type &= ~MOVETYPE_JETPACK;
    }

    ent->m_motion_state.m_input_velocity = local_input_velocity;

    if (ent->m_char->m_char_data.m_afk &&
        glm::length2(ent->m_motion_state.m_input_velocity) > std::numeric_limits<float>::epsilon())
    {
        if(logMovement().isDebugEnabled() && ent->m_motion_state.m_debug && ent->m_type == EntType::PLAYER)
        {
            qCDebug(logMovement) << "Moving so turning off AFK";
        }

        setAFK(*ent->m_char, false);
        ent->m_entity_update_flags.setFlag(ent->UpdateFlag::AFK);
    }
}

static void entMoveNoCollision(Entity* ent)
{
    ent->m_motion_state.m_is_falling = true;
    ent->m_motion_state.m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    ent->m_motion_state.m_move_time += ent->m_input_state.m_timestep;
    float time_scale = (ent->m_motion_state.m_move_time + 6.0f) / 6.0f;
    time_scale = std::min(time_scale, 50.0f);

    ent->m_entity_data.m_pos += time_scale * ent->m_input_state.m_timestep * ent->m_motion_state.m_input_velocity;

    if (glm::length2(ent->m_motion_state.m_input_velocity) <= std::numeric_limits<float>::epsilon())
    {
        ent->m_motion_state.m_move_time = 0.0f;
    }
}

static void entWalk(Entity* ent)
{
    SurfaceParams surf_params;
    entWorldGetSurface(ent, &surf_params);
    SurfaceParams *surf_mods = getSurfaceModifier(ent);
    entWorldApplySurfMods(surf_mods, &surf_params);
    applySurfModsByFlags(&surf_params, ent->m_motion_state.m_walk_flags);

    int is_sliding = 0;
    if(surf_params.traction < 0.3f && !ent->m_motion_state.m_is_falling)
        is_sliding = 1;

    if(ent->m_motion_state.m_is_bouncing)
        ent->m_motion_state.m_is_bouncing = false;

    checkJump(ent, &surf_params);
    //s_landed_on_ground = 0;     // this should be stored in motion state
    doPhysics(ent, surf_mods);

    /* TODO: what? this part doesn't make sense
    if (ent->m_motion_state.m_is_sliding)
        ent->m_motion_state.m_is_sliding++;
    */

    if(is_sliding && ent->m_motion_state.m_velocity.y <= 0.0f)
    {
        ent->m_motion_state.m_is_sliding = true;
        ent->m_seq_state.set(SeqBitNames::SLIDING);
    }

    /* TODO: s_landed_on_ground is only ever 0?
    if(s_landed_on_ground)
    {
        if ( ent->m_motion_state.m_velocity.y < -2.0f )
        {
            // client does camera shake here. Maybe we do something?
        }

        ent->m_motion_state.m_jump_apex = ent->m_states.current()->m_pos_delta.y;
    }*/

    roundVelocityToZero(&ent->m_motion_state.m_velocity);

    // we set this at the start simulating a tick, so I don't think this is needed
    //ent->m_motion_state.m_last_pos = ent->m_entity_data.m_pos;
}

static bool checkEntColl(Entity* /*ent*/, bool /*bouncing*/)
{
    // not implemented
    return true;
}

static void entMotion(Entity* ent)
{
    if ( ent->m_motion_state.m_is_falling
         || glm::length2(ent->m_motion_state.m_velocity) > std::numeric_limits<float>::epsilon()
         || glm::length2(ent->m_motion_state.m_input_velocity) > std::numeric_limits<float>::epsilon()
         || !(ent->m_move_type & MOVETYPE_WALK)
         || (ent->m_type == EntType::PLAYER && checkEntColl(ent, /*bouncing*/ false)) )
    {
        if (ent->m_move_type & MOVETYPE_NOCOLL)
        {
            entMoveNoCollision(ent);
        }
        else if (ent->m_move_type & MOVETYPE_WALK)
        {
            ent->m_motion_state.m_move_time = 0;

            // TODO: this is temprary spoofing of player movement, remove this when putting in entWorldCollide
            ent->m_motion_state.m_velocity = ent->m_motion_state.m_input_velocity;
            ent->m_entity_data.m_pos += ent->m_motion_state.m_velocity * 2.0f;
            // End of temporary spoofing

            entWalk(ent);
            ent->m_motion_state.m_input_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            ent->m_motion_state.m_is_falling = false;
        }
    }
}

// based on pmotionWithPrediction()
static void playerMotion(Entity* player, glm::vec3* final_input_velocity)
{
    player->m_motion_state.m_input_velocity = *final_input_velocity;

    // will be needed later
    /*
    if (player->m_motion_state.m_landing_recovery_time)
    {
        --player->m_motion_state.m_landing_recovery_time;
    }*/

    entMotion(player);
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

                            // control id 8 signals the end of a tick, so now we can simulate this tick

                            // Save current position to last_pos
                            e.m_motion_state.m_last_pos = e.m_entity_data.m_pos;

                            input_state->m_velocity_scale = csc.data.control_id_8.velocity_scale / 255.0f;

                            e.m_motion_state.m_controls_disabled = csc.data.control_id_8.controls_disabled;

                            // following code is based on playerPredictMotion_betaname() in client
                            entMotionUpdateControlsPrePhysics(&e, &tick_state);
                            entMotionSetInputVelocity(&e, &tick_state);

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
    e.m_entity_update_flags.setFlag(e.UpdateFlag::MOVEMENT);
}

void forceOrientation(Entity &e, glm::vec3 pyr)
{
    e.m_direction = glm::quat(pyr);
    e.m_entity_data.m_orientation_pyr = pyr;
    e.m_force_pos_and_cam = true;
    e.m_entity_update_flags.setFlag(e.UpdateFlag::MOVEMENT);
}

// Move to Sequences or Triggers files later
void addTriggeredMove(Entity &e, uint32_t move_idx, uint32_t delay, uint32_t fx_idx)
{
    TriggeredMove tmove;
    tmove.m_move_idx = move_idx;
    tmove.m_ticks_to_delay = delay;
    tmove.m_trigger_fx_idx = fx_idx;

    e.m_triggered_moves.push_back(tmove);
    e.m_entity_update_flags.setFlag(e.UpdateFlag::ANIMATIONS);
    qCDebug(logAnimations) << "Queueing triggered move:"
                           << move_idx << delay << fx_idx;
}

//! @}
