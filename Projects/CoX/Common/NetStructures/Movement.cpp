/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
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

static glm::mat3    s_identity_matrix = glm::mat3(1.0f);
static int          s_landed_on_ground = 0;
//static CollInfo     s_last_surf;
static int          s_reverse_control_dir[6] = {
    BinaryControl::BACKWARD,
    BinaryControl::FORWARD,
    BinaryControl::RIGHT,
    BinaryControl::LEFT,
    BinaryControl::DOWN,
    BinaryControl::UP,
};

SurfaceParams g_world_surf_params[2] = {
    // traction, friction, bounce, gravity, max_speed
    { 1.00f, 0.45f, 0.01f, 0.065f, 1.00f }, // ground; from client
    { 0.02f, 0.01f, 0.00f, 0.065f, 1.00f }  // air; from client
};


// Private Methods
void entWorldCollide(Entity *ent, SurfaceParams *surface_params);
bool checkHead(Entity *ent, int val);
void checkFeet(Entity &ent, SurfaceParams &surf_params);
int slideWall(Entity *ent, glm::vec3 *bottom, glm::vec3 *top);
void entWorldGetSurface(Entity *ent, SurfaceParams *surf_params);

void roundVelocityToZero(glm::vec3 *vel)
{
    if (std::abs(vel->x) < 0.00001f)
        vel->x = 0;
    if (std::abs(vel->y) < 0.00001f)
        vel->y = 0;
    if (std::abs(vel->z) < 0.00001f)
        vel->z = 0;
}

// This will be deprecated in favor of setVelocity()
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

// TODO: we probably don't need this server side.
// pmotionUpdateControlsPrePhysics(ent, cs, current_timestamp_ms);
void calculateKeypressTime(Entity *ent, InputState *controls, std::chrono::steady_clock::time_point cur_time)
{
    uint16_t press_time_ms;

    unsigned int is_jumping = ent->m_motion_state.m_is_jumping;
    for(int i = 0; !is_jumping && i <= BinaryControl::LAST_BINARY_VALUE; ++i )
    {
        is_jumping |= (controls->m_keypress_time[i] >= 250); // if any control is above 250ms then all below 250 will be set to 250
    }

    for(int ctrl_id = 0; ctrl_id <= BinaryControl::LAST_BINARY_VALUE; ++ctrl_id )
    {
        if(controls->m_controls_disabled && !ent->m_player->m_options.alwaysmobile)
        {
            controls->m_keypress_time[ctrl_id] = 0;
            continue;
        }

        if(!controls->m_control_bits[ctrl_id] || controls->m_control_bits[s_reverse_control_dir[ctrl_id]]) // not pressed or pressed both direction at once
            continue;

        press_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - controls->m_keypress_start[ctrl_id]).count();
        controls->m_keypress_time[ctrl_id] += press_time_ms;

        if ( is_jumping && controls->m_keypress_time[ctrl_id] < 250 )
        {
            controls->m_keypress_time[ctrl_id] = 250;
        }
        else if ( controls->m_keypress_time[ctrl_id] > 1000 )
        {
            controls->m_keypress_time[ctrl_id] = 1000;
        }
    }
}

// TODO: we probably don't need all of this. Maybe just landing recovery?
void pmotionWithPrediction(Entity *ent, InputState *controls, glm::vec3 *new_vel)
{
    controls->m_pos_start = controls->m_pos_end;
    ent->m_states.current()->m_pos_delta = controls->m_pos_end;
    //ent->m_states.current()->m_time_state.m_timestep = 1.0f;
    ent->m_motion_state.m_input_velocity = *new_vel;

    if (controls->m_landing_recovery_time)
        --controls->m_landing_recovery_time;

    //m_states->m_motion = ent->m_motion_state;
    //dump_grid_coll_info = cs->controldebug;

    entMotion(ent, controls);

    //dump_grid_coll_info = 0;
    controls->m_pos_end = ent->m_states.current()->m_pos_delta;
}

// TODO: we probably don't need this server side.
void resetKeypressTime(InputState *controls, std::chrono::steady_clock::time_point curtime)
{
    for (int i = 0; i <= BinaryControl::LAST_BINARY_VALUE; ++i )
    {
        // reset total time if button is released, or reverse button is pressed
        if ( !controls->m_control_bits[i] || controls->m_control_bits[s_reverse_control_dir[i]] )
            controls->m_keypress_time[i] = 0;
        controls->m_keypress_start[i] = curtime;
    }
}

int checkEntColl(Entity *ent, int bouncing)
{
    // Do collision checks
    return 1;
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

    if (state->m_no_collision)
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

            if (i >= BinaryControl::UP && !motion->m_is_flying) // UP or Fly
                control_amounts[i] = (float)(press_time != 0);
            else if (!press_time)
                control_amounts[i] = 0.0f;
            else if (press_time >= 1000)
                control_amounts[i] = 1.0f;
            else if (press_time <= 50 && state->m_control_bits[i])
                control_amounts[i] = 0.0f;
            else if (press_time >= 75)
            {
                if (press_time < 75 || press_time >= 100)
                    control_amounts[i] = (float)(press_time - 100) * 0.004f / 9.0f + 0.6f;
                else
                    control_amounts[i] = std::pow(float(press_time - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
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

        if(e.m_type == EntType::PLAYER
                && glm::length(vel))
            qCDebug(logMovement) << "vel:" << glm::to_string(vel).c_str();

        if (!motion->m_is_flying)
            horiz_vel.y = 0.0f;

        if (vel.z < 0.0f)
            vel_scale_copy *= motion->m_backup_spd;

        if (motion->m_is_stunned)
            vel_scale_copy *= 0.1f;

        // Client returns 0 for negative length vector
        if(glm::length(horiz_vel) <= 0.0f)
            horiz_vel = glm::vec3(0,0,0);
        else
            horiz_vel = glm::normalize(horiz_vel);

        if(e.m_type == EntType::PLAYER
                && glm::length(horiz_vel))
            qCDebug(logMovement) << "horizVel:" << glm::to_string(horiz_vel).c_str();

        if (state->m_speed_scale != 0.0f)
            vel_scale_copy *= state->m_speed_scale;

        motion->m_velocity_scale = vel_scale_copy;
        vel.x = horiz_vel.x * std::fabs(control_amounts[BinaryControl::RIGHT] - control_amounts[BinaryControl::LEFT]);
        vel.z = horiz_vel.z * std::fabs(control_amounts[BinaryControl::FORWARD] - control_amounts[BinaryControl::BACKWARD]);

        if (motion->m_is_flying)
            vel.y = horiz_vel.y * std::fabs(control_amounts[BinaryControl::UP] - control_amounts[BinaryControl::DOWN]);
        else if (e.m_states.previous()->m_control_bits[BinaryControl::UP] && !state->m_control_bits[BinaryControl::UP])
            vel.y = 0.0f;
        else
        {
            vel.y *= glm::clamp(motion->m_jump_height, 0.0f, 1.0f);
            if (!e.m_motion_state.m_is_sliding)
                e.m_motion_state.m_flag_5 = false; // flag_5 moving on y-axis?
        }

    }

    // Movement Flags
    if (motion->m_is_flying)
        e.m_move_type |= MoveType::MOVETYPE_FLY;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_FLY;

    if (motion->m_has_jumppack)
        e.m_move_type |= MoveType::MOVETYPE_JETPACK;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_JETPACK;

    motion->m_input_velocity = vel;

    if(e.m_type == EntType::PLAYER && glm::length(vel))
        qCDebug(logMovement) << "final vel:" << glm::to_string(vel).c_str();

    if (e.m_char->m_char_data.m_afk && motion->m_input_velocity != glm::vec3(0,0,0))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Moving so turning off AFK";

        toggleAFK(*e.m_char);
    }

    // setPlayerVelQuat(&vel, vel_scale_copy); // we don't need this?
    motion->m_velocity = vel;
}

void entMoveNoCollision(Entity *ent)
{
    ent->m_motion_state.m_is_falling = true;
    ent->m_motion_state.m_velocity = glm::vec3(0,0,0);
    ent->m_motion_state.m_move_time += ent->m_states.current()->m_time_state.m_timestep;
    float time_scale = (ent->m_motion_state.m_move_time + 6.0f) / 6.0f;

    if ( time_scale > 50.0f )
        time_scale = 50.0f;

    qCDebug(logMovement) << "m_pos_delta:" << glm::to_string(ent->m_states.current()->m_pos_delta).c_str()
                         << "time_scale:" << time_scale
                         << "timestep_rel:" << ent->m_states.current()->m_time_state.m_timestep // formerly timestep
                         << "velocity:" << glm::to_string(ent->m_motion_state.m_velocity).c_str();

    ent->m_states.current()->m_pos_delta += time_scale * ent->m_states.current()->m_time_state.m_timestep * ent->m_motion_state.m_input_velocity;
    ent->m_motion_state.m_last_pos = ent->m_states.current()->m_pos_delta;

    qCDebug(logMovement) << "m_pos no coll" << glm::to_string(ent->m_motion_state.m_last_pos).c_str();

    if(glm::any(glm::lessThanEqual(ent->m_motion_state.m_input_velocity, glm::vec3(0.001f, 0.001f, 0.001f))))
        ent->m_motion_state.m_move_time = 0;
}

void entWorldGetSurface(Entity *ent, SurfaceParams *surf_params)
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

SurfaceParams *getSurfaceModifier(Entity *e)
{
    static SurfaceParams surf;
    bool is_in_air = (e->m_motion_state.m_is_falling || e->m_motion_state.m_is_flying) != 0;
    surf = e->m_motion_state.m_surf_mods[is_in_air];

    if (e->m_motion_state.m_is_flying || e->m_motion_state.m_is_jumping)
    {
        surf.gravity = 0;
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

void entWorldApplySurfMods(SurfaceParams *surf1, SurfaceParams *surf2)
{
    surf2->traction = surf1->traction * surf2->traction;
    surf2->friction = surf1->friction * surf2->friction;
    surf2->bounce   = surf1->bounce * surf2->bounce;
    surf2->gravity  = surf1->gravity * surf2->gravity;
    surf2->max_speed = surf1->max_speed * surf2->max_speed;
}

void applySurfModsByFlags(SurfaceParams *surf, int walk_flags)
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

void checkJump(Entity *ent, InputState *new_state, SurfaceParams *surf_params)
{
    bool want_jump = false;

    if(ent->m_motion_state.m_jump_time > 0)
        --ent->m_motion_state.m_jump_time;

    if(ent->m_move_type & MoveType::MOVETYPE_FLY)
        return;

    if(ent->m_motion_state.m_input_velocity.y > 0.001f) // formerly: input_vel
    {
        if ((new_state && ent->m_motion_state.m_is_stunned)
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
                ent->m_motion_state.m_jump_height       = ent->m_states.current()->m_pos_delta.y;
                ent->m_motion_state.m_max_jump_height   = new_state ? ent->m_motion_state.m_jump_height * 4.0 : 4.0f;
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
                || ent->m_states.current()->m_pos_delta.y - ent->m_motion_state.m_jump_height >= ent->m_motion_state.m_max_jump_height
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
             && surf_params->gravity != 0.0f
             && ent->m_motion_state.m_is_falling)
    {
        ent->m_motion_state.m_input_velocity.y = 0;
    }
}

void doPhysicsOnce(Entity *ent, SurfaceParams *surf_params)
{
    ent->m_motion_state.m_surf_repulsion = glm::vec3(0,0,0);
    entWorldCollide(ent, surf_params);
    ent->m_motion_state.m_velocity += ent->m_motion_state.m_surf_repulsion;
}

void doPhysics(Entity *ent, SurfaceParams *surf_mods)
{
    float timestep = ent->m_states.current()->m_time_state.m_timestep;
    float mag = glm::length(ent->m_motion_state.m_velocity) * timestep;

    if (mag >= 1.0f || timestep >= 5.0f)
    {
        int numsteps = int32_t((mag + 0.99f) / 1.0f);

        if (numsteps > 5 || timestep >= 5.0f)
            numsteps = 5;

        ent->m_states.current()->m_time_state.m_timestep /= float(numsteps);

        for(int i=0; i<numsteps; ++i)
            doPhysicsOnce(ent, surf_mods);

        ent->m_states.current()->m_time_state.m_timestep *= numsteps;
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

void entWalk(Entity *ent, InputState *new_state)
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

    if(new_state)
        ent->m_motion_state.m_velocity_scale = new_state->m_velocity_scale; // velocity?

    checkJump(ent, new_state, &surf_params);
    s_landed_on_ground = 0;     // this should be stored in motion state
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

    if(s_landed_on_ground)
    {
        if ( ent->m_motion_state.m_velocity.y < -2.0f )
        {
            // client does camera shake here. Maybe we do something?
        }

        ent->m_motion_state.m_jump_apex = ent->m_states.current()->m_pos_delta.y;
    }

    roundVelocityToZero(&ent->m_motion_state.m_velocity);
    ent->m_motion_state.m_last_pos = ent->m_states.current()->m_pos_delta;
}

void entMotion(Entity *ent, InputState *new_state)
{
    QString debugmsg = "entMotion failed to perform";
    if(ent->m_motion_state.m_is_falling
         || (glm::length(ent->m_motion_state.m_velocity))
         || (glm::length(ent->m_motion_state.m_input_velocity))
         || !(ent->m_move_type & MoveType::MOVETYPE_WALK)
         || (ent->m_type == EntType::PLAYER && checkEntColl(ent, 0)))
    {
        if (ent->m_move_type & MoveType::MOVETYPE_NOCOLL)
        {
            debugmsg = "entMotion with no Collision";
            entMoveNoCollision(ent);
        }
        else if(ent->m_move_type & MoveType::MOVETYPE_WALK )
        {
            debugmsg = "entMotion normal";
            ent->m_motion_state.m_move_time = 0;
            entWalk(ent, new_state);
            ent->m_motion_state.m_input_velocity = glm::vec3(0,0,0);
        }
        else
        {
            debugmsg = "entMotion we're not walking";
            ent->m_motion_state.m_is_falling = false;
        }

        //qCDebug(logMovement).noquote() << debugmsg;
    }
}

void old_entWorldCollide(Entity *ent, SurfaceParams *surface)
{
    glm::vec3   top;
    glm::vec3   bottom;
    float       before;

    int try_count = 0;
    if(ent->m_states.current()->m_pos_delta.y > 900.0f)
    {
        float tmp_height = ent->m_motion_state.m_velocity.y;
        ent->m_states.current()->m_pos_delta.y = 900.0f;

        if(tmp_height > 0.0f)
            ent->m_motion_state.m_velocity.y = 0;

        if(ent->m_motion_state.m_is_jumping)
        {
            ent->m_motion_state.m_is_falling = true;
            ent->m_motion_state.m_is_jumping = false;
        }
    }

    while(true)
    {
        ++try_count;
        top = ent->m_motion_state.m_last_pos;
        bottom = ent->m_states.current()->m_pos_delta;

        if(slideWall(ent, &bottom, &top))
        {
            ent->m_motion_state.m_stuck_head = StuckType::STUCK_COMPLETELY;
            ent->m_states.current()->m_pos_delta = ent->m_motion_state.m_last_pos;
        }
        else
        {
            ent->m_motion_state.m_stuck_head = StuckType::STUCK_NONE;
            ent->m_states.current()->m_pos_delta = top;
        }

        before = ent->m_states.current()->m_pos_delta.y;
        checkFeet(*ent, *surface);

        if(std::fabs(ent->m_states.current()->m_pos_delta.y - before) <= 0.00001f )
            break;

        if(!checkHead(ent, 0))
            return;

        if(surface->gravity != 0.0f || try_count != 1)
        {
            if ( ent->m_motion_state.m_velocity.y > 0.0f )
                ent->m_motion_state.m_velocity.y = 0;

            ent->m_motion_state.m_stuck_head = StuckType::STUCK_SLIDE;
            ent->m_states.current()->m_pos_delta = ent->m_motion_state.m_last_pos;
            return;
        }

        ent->m_states.current()->m_pos_delta.y = ent->m_motion_state.m_last_pos.y;
    }

    if(ent->m_motion_state.m_stuck_head == StuckType::STUCK_COMPLETELY && checkHead(ent, 1))
    {
        before = ent->m_states.current()->m_pos_delta.y;
        checkFeet(*ent, *surface);

        if(std::fabs(ent->m_states.current()->m_pos_delta.y - before) > 0.00001f)
            ent->m_states.current()->m_pos_delta = ent->m_motion_state.m_last_pos;
    }
}

void motionSetSequence(Entity *ent, InputState *new_state)
{
    float       z_axis              = 0.0f;
    float       guess_timestep      = 0.0f;
    bool        is_moving_backwards;
    signed int  negative_control_delta;

    is_moving_backwards = new_state->m_velocity_scale > 1.0f;
    negative_control_delta = 0;

    if(ent->m_motion_state.m_is_flying)
        ent->m_seq_state.set(SeqBitNames::FLY);
    if(ent->m_motion_state.m_is_stunned )
        ent->m_seq_state.set(SeqBitNames::STUN);
    if(ent->m_motion_state.m_has_jumppack)
        ent->m_seq_state.set(SeqBitNames::JETPACK);
    if(ent->m_motion_state.m_is_jumping)
        ent->m_seq_state.set(SeqBitNames::JUMP);
    if(ent->m_motion_state.m_is_sliding)
        ent->m_seq_state.set(SeqBitNames::SLIDING);
    if(ent->m_motion_state.m_is_bouncing)
        ent->m_seq_state.set(SeqBitNames::BOUNCING);

    if(ent->m_motion_state.m_is_falling)
    {
        ent->m_seq_state.set(SeqBitNames::AIR);
        bool sliding = ent->m_motion_state.m_is_sliding;

        if ( !sliding && ent->m_motion_state.m_velocity.y < -0.6f && ent->m_motion_state.m_jump_apex - ent->m_states.current()->m_pos_delta.y > 3.5f )
            ent->m_seq_state.set(SeqBitNames::BIGFALL);
    }
    else if (ent->m_motion_state.m_has_headpain)
        ent->m_seq_state.set(SeqBitNames::HEADPAIN);

    if(ent->m_states.current()->m_no_collision
            && ent->m_player->m_options.alwaysmobile
            && (ent->m_motion_state.m_controls_disabled || !ent->m_motion_state.m_has_headpain))
    {
        //int x_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::RIGHT] - ent->m_states.current()->m_keypress_time[BinaryControl::LEFT];
        int z_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::UP] - ent->m_states.current()->m_keypress_time[BinaryControl::DOWN];
        //int y_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::FORWARD] - ent->m_states.current()->m_keypress_time[BinaryControl::BACKWARD];

        if(ent->m_player->m_options.no_strafe)
        {
            if(ent->m_motion_state.m_velocity.x != 0.0f && ent->m_motion_state.m_input_velocity.z >= 0.0f)
            {
                negative_control_delta = 1;
                if ( is_moving_backwards )
                    ent->m_seq_state.set(SeqBitNames::BACKWARD);
                else
                    ent->m_seq_state.set(SeqBitNames::FORWARD);
            }
        }
        else if(ent->m_motion_state.m_input_velocity.x != 0.0f)
        {
            negative_control_delta = 1;
            if(ent->m_motion_state.m_input_velocity.x <= 0.0f)
                ent->m_seq_state.set(SeqBitNames::STEPLEFT);
            else
                ent->m_seq_state.set(SeqBitNames::STEPRIGHT);
        }

        if(z_mag_delta > 0)
        {
            if (ent->m_motion_state.m_is_flying)
                ent->m_seq_state.set(SeqBitNames::JUMP);
        }

        if (ent->m_motion_state.m_input_velocity.z != 0.0f )
        {
            if(is_moving_backwards)
                z_axis = -ent->m_motion_state.m_input_velocity.z;
            else
                z_axis = ent->m_motion_state.m_input_velocity.z;

            negative_control_delta = 1;
            if ( z_axis <= 0.0f )
                ent->m_seq_state.set(SeqBitNames::BACKWARD);
            else
                ent->m_seq_state.set(SeqBitNames::FORWARD);
        }

        if(negative_control_delta)
        {
            guess_timestep = (new_state->m_move_time <= 900) ? 3 : 0;
        }
        else if(guess_timestep > 2.0f)
        {
            guess_timestep = 2.0f;
        }

        if (guess_timestep > 0.0f && guess_timestep <= 2.0f)
        {
            if(ent->m_seq_state.isSet(SeqBitNames::COMBAT)
                 || ent->m_motion_state.m_is_falling
                 || ent->m_motion_state.m_is_flying)
            {
                guess_timestep = 0;
            }
            else
            {
                guess_timestep -= ent->m_states.current()->m_time_state.m_timestep;
                ent->m_seq_state.set(SeqBitNames::FORWARD);
                ent->m_seq_state.set(SeqBitNames::IDLE);
            }
        }

        /*
        if ( my_IsDevelopmentMode() )
        {
            for ( i = 0; i < 10; ++i )
            {
                if(cs->atest_par[i])
                    ent->m_seq_state->set(SeqBitNames(uint32_t(SeqBitNames::TEST0)+i));
            }
        }
        */
    }
}

void entWorldCollide(Entity *ent, SurfaceParams *surface_params)
{
    glm::vec3 dv;
    SurfaceParams surf;
    glm::mat3 surf_mat;
    glm::vec3 up(0,1,0); // formerly 0,0,1?? why z?
    glm::vec3 final_velocity(0,0,0);
    glm::vec3 xvel(0,0,0);
    glm::vec3 tvel(0,0,0);
    glm::vec3 surf_normal = ent->m_motion_state.m_surf_normal;
    float fractional_vel = 0.0f;

    entWorldGetSurface(ent, &surf);
    entWorldApplySurfMods(surface_params, &surf);

    if(ent->m_motion_state.m_walk_flags & CollFlags::COLL_SURFACE_FLAGS) {
        applySurfModsByFlags(&surf, ent->m_motion_state.m_walk_flags);
    }
    else
    {
        if(surf.friction > 1.0f)
            surf.friction = 1.0f;
        if(surf.traction > 1.0f)
            surf.traction = 1.0f;
    }

    float friction_rel  = 1.0f - std::pow(1.0f - surf.friction, ent->m_states.current()->m_time_state.m_timestep);
    float max_speed     = surf.max_speed;
    float traction_rel  = 1.0f - std::pow(1.0f - surf.traction, ent->m_states.current()->m_time_state.m_timestep);

    if(ent->m_motion_state.m_velocity_scale != 0.0f)
        max_speed = surf.max_speed * ent->m_motion_state.m_velocity_scale;

    if(surf.gravity != 0.0f)
    {
        float gravity_rel = surf.gravity;
        if (ent->m_motion_state.m_velocity.y < 0)
            gravity_rel = gravity_rel * 1.5f;
        tvel.y = -(gravity_rel * ent->m_states.current()->m_time_state.m_timestep);
    }

    if(ent->m_motion_state.m_flag_14)
    {
        glm::vec3 *p_surf_normal = &ent->m_motion_state.m_surf_normal2;
        if (ent->m_motion_state.m_field_88 == 1
                && ent->m_motion_state.m_surf_normal2.y < surf_normal.y
                && ent->m_motion_state.m_velocity.y <= 0.0f)
        {
            p_surf_normal = &surf_normal;
        }

        if (glm::dot(ent->m_motion_state.m_surf_normal3, *p_surf_normal) < -0.7f)
        {
            ent->m_motion_state.m_is_falling = false;
            ent->m_motion_state.m_has_jumppack = true;
            traction_rel = 1.0;
        }

        ent->m_motion_state.m_surf_normal3 = *p_surf_normal;
        // below: f_reflect(p_surf_normal, &dv, &tvel);
        tvel += glm::reflect(surf_normal, tvel); // dv;

        if (ent->m_motion_state.m_is_flying || p_surf_normal->y <= 0.0f)
        {
            final_velocity = traction_rel * glm::length(ent->m_motion_state.m_input_velocity) * ent->m_motion_state.m_input_velocity;
        }
        else
        {
            final_velocity = ent->m_motion_state.m_input_velocity;
            final_velocity.y = -((ent->m_motion_state.m_input_velocity.x * p_surf_normal->x + p_surf_normal->z * ent->m_motion_state.m_input_velocity.z) / p_surf_normal->y);
            float vel_with_traction = glm::length(ent->m_motion_state.m_input_velocity) * traction_rel;
            if (final_velocity.y < 0.0f)
            {
                float tmp_vel = 1.0f - glm::dot(tvel, final_velocity) / (glm::length(tvel) * glm::length(final_velocity)) * (1.0f - p_surf_normal->y);
                max_speed = max_speed / std::max(0.1f, tmp_vel);
            }
            else
            {
                vel_with_traction *= p_surf_normal->y;
            }
            final_velocity *= vel_with_traction;
        }

        float traction_temp = std::min((1.0f - traction_rel) * 4.0f, 1.0f);
        tvel = tvel * (traction_temp * 0.5f);

        camLookAt(p_surf_normal, &surf_mat);

        glm::mat3 surf_mat_inv(surf_mat);
        glm::transpose(surf_mat_inv);
        xvel = ent->m_motion_state.m_velocity * surf_mat_inv;

        if (xvel.z > 0.0f)
            dv = ent->m_motion_state.m_velocity;
        else
        {
            glm::vec3 dvx = glm::reflect(up, xvel);
            dvx.z = dvx.z * surf.bounce;
            dv = dvx * surf_mat;
        }

        float dot_vel = 0.0f;
        bool run_once = true;
        if (final_velocity.x != 0.0f || final_velocity.y != 0.0f || final_velocity.z != 0.0f)
        {
            run_once = false;
            dot_vel = glm::dot(dv, final_velocity);
        }

        if (!(ent->m_motion_state.m_is_falling || ent->m_motion_state.m_is_flying))
        {
            ent->m_motion_state.m_coll_surf_flags = ent->m_motion_state.m_walk_flags;
            ent->m_motion_state.m_flag_15 = true;
        }

        float surf_rel = std::min(1.0f, float(traction_rel * 0.5f + friction_rel));
        if ( run_once )
        {
            dv = (1 - friction_rel)*dv + final_velocity;

        }
        else if (dot_vel >= 0.0f)
        {
            float final_magnitude = 0.0f;
            fractional_vel = 1.0f / glm::length2(final_velocity);
            float vel_with_surf = glm::dot(-(surf_rel * dv), final_velocity) * fractional_vel;
            dv = (1 - surf_rel)*dv - final_velocity * vel_with_surf;
            xvel = final_velocity * glm::dot(dv, final_velocity) * fractional_vel;
            bool run_once = true;

            if (glm::length2(xvel) > max_speed * max_speed)
            {
                run_once = false;
                final_magnitude = glm::length(xvel);
            }

            dv += final_velocity;
            xvel = final_velocity * (glm::dot(dv, final_velocity) * fractional_vel);
            if (glm::length2(xvel) > max_speed * max_speed)
            {
                float final_speed = max_speed;
                if (!run_once)
                    final_speed = (final_magnitude - max_speed) * (1.0f - friction_rel) + max_speed;
                dv = dv + xvel*((final_speed / glm::length(xvel)) - 1);
            }
        }
        else
            dv = (1 - surf_rel) *dv + final_velocity;

        ent->m_motion_state.m_velocity = tvel + dv;

    }
    else
    {
        if(ent->m_motion_state.m_flag_15 && !(ent->m_motion_state.m_coll_surf_flags & COLL_SURFACE_FLAGS) && ent->m_motion_state.m_velocity.y > 0.0f)
            ent->m_motion_state.m_velocity.y = ent->m_motion_state.m_has_jumppack ? std::min(1.0f, ent->m_motion_state.m_velocity.y) : 0;

        ent->m_motion_state.m_is_slowed = false;
        ent->m_motion_state.m_flag_15 = false;
        ent->m_motion_state.m_surf_normal3 = glm::vec3(0,0,0);
        dv = ent->m_motion_state.m_velocity;
        float modified_vel = glm::length(ent->m_motion_state.m_input_velocity) * traction_rel;
        final_velocity = modified_vel * ent->m_motion_state.m_input_velocity;

        float y_vel_rel = 0.0f;
        if (ent->m_motion_state.m_is_flying)
            y_vel_rel = 0.0;
        else
        {
            y_vel_rel = ent->m_motion_state.m_velocity.y;
            dv.y = 0.0;
            final_velocity.y = 0.0;
        }

        if (final_velocity.x == 0.0f && final_velocity.y == 0.0f && final_velocity.z == 0.0f)
        {
            dv = (1 - friction_rel)*dv + final_velocity;
        }
        else if (glm::dot(dv, final_velocity) < 0.0f)
        {
            float force = std::min(1.0f, traction_rel * 0.5f + friction_rel);
            dv = (1 - force) * dv + final_velocity;
        }
        else
        {
            friction_rel = std::min(1.0f, float(traction_rel * 0.5f + friction_rel));
            up = -friction_rel * dv;
            fractional_vel = 1.0f / glm::length2(final_velocity);
            xvel = final_velocity * glm::dot(up, final_velocity) * fractional_vel;
            dv = up - xvel + dv;
            float dot_vel = glm::dot(dv, final_velocity) * fractional_vel;

            float final_magnitude = 0.0f;
            bool exceeded_max_spd = true;
            if (glm::length2(dot_vel * final_velocity) > max_speed * max_speed)
            {
                exceeded_max_spd = false;
                final_magnitude = glm::length(dot_vel * final_velocity);
            }

            dv += final_velocity;
            xvel = final_velocity * (glm::dot(dv, final_velocity) * fractional_vel);
            if (glm::length2(xvel) > max_speed * max_speed)
            {
                dv -= xvel;
                float final_speed = max_speed;
                if (!exceeded_max_spd)
                    final_speed = (final_magnitude - max_speed) * (1.0f - friction_rel) + max_speed;
                dv += xvel * final_speed / glm::length(xvel);
            }
        }

        ent->m_motion_state.m_velocity = dv + tvel;
        ent->m_motion_state.m_velocity.y += y_vel_rel;
    }

    if (ent->m_motion_state.m_is_jumping)
        ent->m_motion_state.m_velocity.y = ent->m_motion_state.m_input_velocity.y;

    ent->m_states.current()->m_pos_delta = ent->m_motion_state.m_last_pos + ent->m_motion_state.m_velocity * ent->m_states.current()->m_time_state.m_timestep;
    checkEntColl(ent, 1);
    old_entWorldCollide(ent, &surf);

    if (ent->m_motion_state.m_is_flying
            || ent->m_motion_state.m_velocity.y >= 0.0f
            || !(ent->m_states.current()->m_pos_delta == ent->m_motion_state.m_last_pos)
            || ent->m_motion_state.m_surf_normal2.y >= 0.5f)
    {
        ent->m_motion_state.m_flag_14 = true; // s_last_surf.ctri != nullptr;
        //ent->m_motion_state.m_surf_normal2 = s_last_surf.mat.row2;
    }
    else
    {
        ent->m_motion_state.m_surf_normal = glm::vec3(0,1,0);
        ent->m_motion_state.m_is_falling =  false;
        ent->m_motion_state.m_has_jumppack =  false;
        ent->m_motion_state.m_flag_14 =  true;
        ent->m_motion_state.m_velocity.y = 0;
    }

    ent->m_motion_state.m_last_pos = ent->m_states.current()->m_pos_delta;
    //ent->m_motion_state.m_walk_flags = s_last_surf.ctri ? s_last_surf.ctri->flags : 0;
}

bool checkHead(Entity */*ent*/, int /*val*/)
{
    /*
    CollInfo dest; // collision info

    glm::vec3 finish = ent->m_states.current()->m_pos_delta + glm::vec3(0, 6, 0);
    glm::vec3 start = ent->m_states.current()->m_pos_delta + glm::vec3(0, 1.5f, 0);
    int coll_flag = CollFlags::COLL_NOTSELECTABLE | CollFlags::COLL_CYLINDER | CollFlags::COLL_DISTFROMSTART;

    if(!s_coll_entblocker)
        coll_flag |= CollFlags::COLL_ENTBLOCKER;

    if(!collGrid(nullptr, &start, &finish, &dest, 1.0, coll_flag))
        return false;

    if(val)
    {
        float dist = ent->m_states.current()->m_pos_delta.y + 6.0f - dest.mat.TranslationPart.y;

        if ( dist > 1.5f )
        {
            ent->m_states.current()->m_pos_delta = ent->m_motion_state.m_last_pos;
            return true;
        }

        ent->m_states.current()->m_pos_delta.y += dist;
    }
    */

    return true;
}

void checkFeet(Entity &/*ent*/, SurfaceParams &/*surf_params*/)
{
    // TODO: requires collision
}

int slideWall(Entity */*ent*/, glm::vec3 */*bottom*/, glm::vec3 */*top*/)
{
    // TODO: requires collision
    return 0;
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
    e.m_update_pos_and_cam = true;
}

void forceOrientation(Entity &e, glm::vec3 pyr)
{
    e.m_direction = glm::quat(pyr);
    e.m_entity_data.m_orientation_pyr = pyr;
    e.m_update_pos_and_cam = true;
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
