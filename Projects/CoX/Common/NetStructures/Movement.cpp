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
#include "Servers/MapServer/DataHelpers.h"
#include "Common/GameData/CoHMath.h"
#include "Common/GameData/seq_definitions.h"
#include "Common/GameData/playerdata_definitions.h"
#include "Logging.h"

#include <glm/gtx/vector_query.hpp>
#include <glm/ext.hpp>

static glm::mat3    s_identity_matrix = glm::mat3(1.0f);
static int          s_landed_on_ground = 0;

SurfaceParams g_world_surf_params[2] = {
    // traction, friction, bounce, gravity, max_speed
    { 1.00f, 0.45f, 0.01f, 0.065f, 1.00f }, // ground; from client
    { 0.02f, 0.01f, 0.00f, 0.065f, 1.00f }  // air; from client
};

/* can replace with: glm::vec3 reflect = glm::reflect(incident, norm);
static void f_reflect(glm::vec3 *norm, glm::vec3 *reflect, glm::vec3 *incident)
{
    glm::vec3 working_vec   = *incident;
    float normalized_vec    = working_vec.normalize(); // glm::normalize(working_vec);
    float dot_product       = norm->dot(- working_vec);
    // update reflection
    *reflect = (2*dot_product * *norm + working_vec) * normalized_vec;
}
*/

void roundVelocityToZero(glm::vec3 *vel)
{
    if (std::abs(vel->x) < 0.00001f)
        vel->x = 0;
    if (std::abs(vel->y) < 0.00001f)
        vel->y = 0;
    if (std::abs(vel->z) < 0.00001f)
        vel->z = 0;
}

void processDirectionControl(InputState *next_state, uint8_t dir, int prev_time, int press_release)
{
    float delta = 0.0f;

    if(press_release)
        delta = 1.0f;

    qCDebug(logInput, "Pressed dir: %s \t prev_time: %d \t press_release: %d", control_name[dir], prev_time, press_release);
    switch(dir)
    {
        case 0: next_state->m_pos_delta[2] = delta; break;    // FORWARD
        case 1: next_state->m_pos_delta[2] = -delta; break;   // BACKWARD
        case 2: next_state->m_pos_delta[0] = -delta; break;   // LEFT
        case 3: next_state->m_pos_delta[0] = delta; break;    // RIGHT
        case 4: next_state->m_pos_delta[1] = delta; break;    // UP
        case 5: next_state->m_pos_delta[1] = -delta; break;   // DOWN
    }

    switch(dir)
    {
        case 0:
        case 1: next_state->m_pos_delta_valid[2] = true; break;
        case 2:
        case 3: next_state->m_pos_delta_valid[0] = true; break;
        case 4:
        case 5: next_state->m_pos_delta_valid[1] = true; break;
    }
}

void setVelocity(Entity &e)
{
    float       control_vals[6] = {0};
    glm::vec3   horiz_vel       = {0, 0, 0};
    float       max_press_time  = 0.0f;
    float       press_time      = 100.0f;
    glm::vec3   vel             = {0, 0, 0};

    if (e.m_states.current()->m_no_collision)
        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;

    if(e.m_states.current()->m_no_collision
            && e.m_states.current()->m_autorun
            && (e.m_motion_state.m_controls_disabled || e.m_motion_state.m_has_headpain))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Input Vel is <0, 0, 0>. No coll?" << e.m_states.current()->m_no_collision;

        e.m_velocity = vel;
    }
    else
    {
        for (int i = BinaryControl::FORWARD; i < BinaryControl::LAST_BINARY_VALUE; ++i)
        {
            press_time = e.m_states.current()->m_keypress_time[i]*30;
            //qCDebug(logMovement) << "keypress_time" << i << e.inp_state.m_keypress_time[i];
            max_press_time = std::max(press_time, max_press_time);
            if (i >= BinaryControl::UP && !e.m_motion_state.m_is_flying) // UP or Fly
                control_vals[i] = (float)(press_time != 0);
            else if (!press_time)
                control_vals[i] = 0.0f;
            else if (press_time >= 1000)
                control_vals[i] = 1.0f;
            else if (press_time <= 50 && e.m_states.previous()->m_control_bits[i])
                control_vals[i] = 0.0f;
            else if (press_time < 75)
                control_vals[i] = 0.2f;
            else if (press_time < 100)
                control_vals[i] = std::pow(float(press_time - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
            else
                control_vals[i] = (float)(press_time - 100) * 0.004f / 9.0f + 0.6f;

            //qCDebug(logMovement) << "control_vals:" << i << control_vals[i];
        }

        e.m_states.current()->m_move_time = max_press_time;
        vel.x = control_vals[BinaryControl::RIGHT] - control_vals[BinaryControl::LEFT];
        vel.y = control_vals[BinaryControl::UP] - control_vals[BinaryControl::DOWN];
        vel.z = control_vals[BinaryControl::FORWARD] - control_vals[BinaryControl::BACKWARD];
        vel.x = vel.x * e.m_speed.x;
        vel.y = vel.y * e.m_speed.y;

        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "vel:" << glm::to_string(vel).c_str();

        horiz_vel = vel;

        if (!e.m_motion_state.m_is_flying)
            horiz_vel.y = 0.0f;

        if (vel.z < 0.0f)
            e.m_states.current()->m_velocity_scale *= e.m_motion_state.m_backup_spd;

        if (e.m_motion_state.m_is_stunned)
            e.m_states.current()->m_velocity_scale *= 0.1f;

        if(horiz_vel.length() <= 0.0f)
            horiz_vel = glm::normalize(horiz_vel);

        if (e.m_states.current()->m_speed_scale != 0.0f)
            e.m_states.current()->m_velocity_scale *= e.m_states.current()->m_speed_scale;

        vel.x = horiz_vel.x * std::fabs(control_vals[BinaryControl::RIGHT] - control_vals[BinaryControl::LEFT]);
        vel.z = horiz_vel.z * std::fabs(control_vals[BinaryControl::FORWARD] - control_vals[BinaryControl::BACKWARD]);

        if (e.m_motion_state.m_is_flying)
            vel.y = horiz_vel.y * std::fabs(control_vals[BinaryControl::UP] - control_vals[BinaryControl::DOWN]);
        else if (e.m_states.previous()->m_control_bits[BinaryControl::UP])
            vel.y = 0.0f;
        else
        {
            vel.y *= glm::clamp(e.m_motion_state.m_jump_height, 0.0f, 1.0f);
            //if (!e.m_is_sliding)
                //ent->motion.flag_5 = false; // flag_5 moving on y-axis?
        }

        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "horizVel:" << glm::to_string(horiz_vel).c_str();
    }

    // Movement Flags
    if (e.m_motion_state.m_is_flying)
        e.m_move_type |= MoveType::MOVETYPE_FLY;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_FLY;

    if (e.m_motion_state.m_has_jumppack)
        e.m_move_type |= MoveType::MOVETYPE_JETPACK;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_JETPACK;

    e.m_velocity = vel; // motion->inp_vel = vel;

    if (e.m_char->m_char_data.m_afk && e.m_velocity != glm::vec3(0,0,0))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Moving so turning off AFK";

        toggleAFK(*e.m_char);
    }
}

void my_entMoveNoColl(Entity *ent)
{
    ent->m_motion_state.m_is_falling = true;
    ent->m_motion_state.m_velocity = glm::vec3(0,0,0);
    ent->m_motion_state.m_move_time += ent->m_states.current()->m_time_state.m_timestep;
    float time_scale = (ent->m_motion_state.m_move_time + 6.0f) / 6.0f;

    if ( time_scale > 50.0f )
        time_scale = 50.0f;

    qCDebug(logMovement) << "BEFORE: m_pos_delta:" << glm::to_string(ent->m_states.current()->m_pos_delta).c_str()
                         << "time_scale:" << time_scale
                         << "timestamp:" << ent->m_states.current()->m_time_state.m_timestep
                         << "velocity:" << glm::to_string(ent->m_velocity).c_str();

    ent->m_states.current()->m_pos_delta += time_scale * ent->m_states.current()->m_time_state.m_timestep * ent->m_velocity/255; // formerly inp_vel
    ent->m_motion_state.m_last_pos = ent->m_states.current()->m_pos_delta; // save pos_delta to motion_state instead?

    qCDebug(logMovement) << "m_pos no coll" << glm::to_string(ent->m_entity_data.m_pos).c_str();

    if(glm::any(glm::lessThanEqual(ent->m_velocity/255, glm::vec3(0.001f, 0.001f, 0.001f))))
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

void entWorldApplySurfMods(SurfaceParams *surf1, SurfaceParams *surf2)
{
    surf2->traction = surf1->traction * surf2->traction;
    surf2->friction = surf1->friction * surf2->friction;
    surf2->bounce   = surf1->bounce * surf2->bounce;
    surf2->gravitational_constant  = surf1->gravitational_constant * surf2->gravitational_constant;
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

    if(ent->m_velocity.y > 0.001f) // formerly: input_vel
    {
        if ((new_state && ent->m_motion_state.m_is_stunned)
                || (!ent->m_motion_state.m_has_jumppack && ent->m_motion_state.m_surf_normal.y <= 0.3f))
        {
            ent->m_velocity.y = 0; // formerly: input_vel
        }
        else
            want_jump = true;
    }

    if (ent->m_velocity.y > 0.0f && ent->m_motion_state.m_has_jumppack
         && (!want_jump && !(ent->m_move_type & MoveType::MOVETYPE_JETPACK)))
    {
        ent->m_motion_state.m_has_jumppack = false;
        ent->m_velocity.y -= std::min(1.5f, ent->m_velocity.y) * 0.5f;
    }

    bool not_moving_on_yaxis = !(ent->m_motion_state.m_is_falling
                                 || ent->m_motion_state.m_is_flying
                                 || ent->m_motion_state.m_jump_time > 0
                                 || ent->m_move_type & MoveType::MOVETYPE_JETPACK);

    if (!ent->m_motion_state.m_is_jumping && want_jump)
    {
        if (!not_moving_on_yaxis || ent->m_motion_state.m_flag_5)
            ent->m_velocity.y = 0; // formerly: inp_vel.y
        else
        {
            ent->m_motion_state.m_is_jumping        = true;
            ent->m_motion_state.m_is_bouncing       = true;
            ent->m_motion_state.m_jump_height       = ent->m_motion_state.m_jump_height;        // formerly: ent->mat4.TranslationPart.y
            ent->m_motion_state.m_max_jump_height   = ent->m_motion_state.m_jump_height * 4.0;  // formerly: cur_state ? cur_state.jump_height * 4.0 : 4.0f
            ent->m_motion_state.m_jump_time         = 15;
            ent->m_motion_state.m_has_jumppack      = true;
            ent->m_motion_state.m_flag_5            = true;
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
        ent->m_seq_state.setVal(SeqBitNames::SB_JETPACK, ent->m_motion_state.m_is_jumping);
    }
    else if (ent->m_velocity.y != 0.0f /* formerly: input_vel */
             && surf_params->gravitational_constant != 0.0f
             && ent->m_motion_state.m_is_falling)
    {
        ent->m_velocity.y = 0; // formerly: input_vel
    }
}

void doPhysicsOnce(Entity *ent, SurfaceParams *surf_params)
{
    ent->m_motion_state.m_surf_repulsion = glm::vec3(0,0,0);
    //entWorldCollide(ent, surf_params);
    ent->m_velocity += ent->m_motion_state.m_surf_repulsion;
}

void doPhysics(Entity *ent, SurfaceParams *surf_mods)
{
    float timestep = ent->m_states.current()->m_time_state.m_timestep;
    float mag = ent->m_velocity.length() * timestep;

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
        if (ent->m_velocity.y > 0.0f && ent->m_velocity.y < 0.3f)
            ent->m_seq_state.set(SeqBitNames::SB_APEX);

        if(ent->m_velocity.y < 0.0f)
        {
            if(!(ent->m_move_type & MoveType::MOVETYPE_JETPACK))
                ent->m_motion_state.m_is_jumping = false; // no longer counted as jump, we're falling

            ent->m_seq_state.setVal(SeqBitNames::SB_JUMP, false);
        }

        if(ent->m_motion_state.m_is_falling)
            ent->m_seq_state.set(SeqBitNames::SB_AIR);
    }
    if(ent->m_motion_state.m_is_jumping)
    {
        if(ent->m_move_type & MoveType::MOVETYPE_JETPACK)
            ent->m_seq_state.set(SeqBitNames::SB_JETPACK);
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
        ent->m_states.current()->m_velocity_scale = new_state->m_velocity_scale; // velocity?

    checkJump(ent, new_state, &surf_params);
    s_landed_on_ground = 0;
    doPhysics(ent, surf_mods);

    /* TODO: what? this part doesn't make sense
    if (ent->m_motion_state.m_is_sliding)
        ent->m_motion_state.m_is_sliding++;
    */

    if(is_sliding && ent->m_velocity.y <= 0.0f)
    {
        ent->m_motion_state.m_is_sliding = true;
        ent->m_seq_state.set(SeqBitNames::SB_SLIDING);
    }

    if(s_landed_on_ground)
    {
        if(ent->m_velocity.y < -2.0f)
        {
            float falling_vel = std::min(2.0f, -ent->m_velocity.y - 2.0f) * 0.2f;
        }
        ent->m_motion_state.m_jump_apex = ent->m_states.current()->m_pos_delta.y;
    }
    roundVelocityToZero(&ent->m_velocity);
    // save pos_delta to motion_state?
    //ent->m_motion_state.m_last_pos = ent->m_states.current()->m_pos_delta;
}

void entMotion(Entity *ent, InputState *new_state)
{
    // Removed additional (ent->m_states.current()->m_velocity_scale == glm::vec3(0,0,0))
    if(ent->m_motion_state.m_is_walking
         || (ent->m_velocity == glm::vec3(0,0,0))
         || !(ent->m_move_type & MoveType::MOVETYPE_WALK)
         || (ent->m_type == EntType::PLAYER/* && checkEntColl(ent, 0)*/))
    {
        if (ent->m_move_type & MoveType::MOVETYPE_NOCOLL)
        {
            qCDebug(logMovement)<< "entMotion with no Collision";
            my_entMoveNoColl(ent);
        }
        else if(ent->m_move_type & MoveType::MOVETYPE_WALK )
        {
            qCDebug(logMovement)<< "entMotion normal";
            ent->m_states.current()->m_time_state.m_timestep = 0; // move_time
            entWalk(ent, new_state);
            ent->m_velocity = glm::vec3(0,0,0); // formerly inp_vel
        }
        else
        {
            qCDebug(logMovement)<< "entMotion we're not walking";
            ent->m_motion_state.m_is_walking = false;
        }

        qCDebug(logMovement)<< "entMotion failed to perform";
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
        ent->m_seq_state.set(SeqBitNames::SB_FLY);
    if(ent->m_motion_state.m_is_stunned )
        ent->m_seq_state.set(SeqBitNames::SB_STUN);
    if(ent->m_motion_state.m_has_jumppack)
        ent->m_seq_state.set(SeqBitNames::SB_JETPACK);
    if(ent->m_motion_state.m_is_jumping)
        ent->m_seq_state.set(SeqBitNames::SB_JUMP);
    if(ent->m_motion_state.m_is_sliding)
        ent->m_seq_state.set(SeqBitNames::SB_SLIDING);
    if(ent->m_motion_state.m_is_bouncing)
        ent->m_seq_state.set(SeqBitNames::SB_BOUNCING);

    if(ent->m_motion_state.m_is_falling)
    {
        ent->m_seq_state.set(SeqBitNames::SB_AIR);
        bool sliding = ent->m_motion_state.m_is_sliding;

        if ( !sliding && ent->m_velocity.y < -0.6f && ent->m_motion_state.m_jump_apex - ent->m_states.current()->m_pos_delta.y > 3.5f )
            ent->m_seq_state.set(SeqBitNames::SB_BIGFALL);
    }
    else if (ent->m_motion_state.m_has_headpain)
        ent->m_seq_state.set(SeqBitNames::SB_HEADPAIN);

    if(ent->m_states.current()->m_no_collision
            && ent->m_player->m_options.alwaysmobile
            && (ent->m_motion_state.m_controls_disabled || !ent->m_motion_state.m_has_headpain))
    {
        //int x_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::RIGHT] - ent->m_states.current()->m_keypress_time[BinaryControl::LEFT];
        int z_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::UP] - ent->m_states.current()->m_keypress_time[BinaryControl::DOWN];
        //int y_mag_delta = ent->m_states.current()->m_keypress_time[BinaryControl::FORWARD] - ent->m_states.current()->m_keypress_time[BinaryControl::BACKWARD];

        if(ent->m_player->m_options.no_strafe)
        {
            if(ent->m_velocity.x != 0.0f && ent->m_velocity.z >= 0.0f) // formerly: inp_vel
            {
                negative_control_delta = 1;
                if ( is_moving_backwards )
                    ent->m_seq_state.set(SeqBitNames::SB_BACKWARD);
                else
                    ent->m_seq_state.set(SeqBitNames::SB_FORWARD);
            }
        }
        else if(ent->m_velocity.x != 0.0f) // formerly: inp_vel
        {
            negative_control_delta = 1;
            if(ent->m_velocity.x <= 0.0f) // formerly: inp_vel
                ent->m_seq_state.set(SeqBitNames::SB_STEPLEFT);
            else
                ent->m_seq_state.set(SeqBitNames::SB_STEPRIGHT);
        }

        if(z_mag_delta > 0)
        {
            if (ent->m_motion_state.m_is_flying)
                ent->m_seq_state.set(SeqBitNames::SB_JUMP);
        }

        if (ent->m_velocity.z != 0.0f ) // formerly: inp_vel
        {
            if(is_moving_backwards)
                z_axis = -ent->m_velocity.z; // formerly: inp_vel
            else
                z_axis = ent->m_velocity.z; // formerly: inp_vel

            negative_control_delta = 1;
            if ( z_axis <= 0.0f )
                ent->m_seq_state.set(SeqBitNames::SB_BACKWARD);
            else
                ent->m_seq_state.set(SeqBitNames::SB_FORWARD);
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
            if(ent->m_seq_state.isSet(SeqBitNames::SB_COMBAT)
                 || ent->m_motion_state.m_is_falling
                 || ent->m_motion_state.m_is_flying)
            {
                guess_timestep = 0;
            }
            else
            {
                guess_timestep -= ent->m_states.current()->m_time_state.m_timestep;
                ent->m_seq_state.set(SeqBitNames::SB_FORWARD);
                ent->m_seq_state.set(SeqBitNames::SB_IDLE);
            }
        }

        /*
        if ( my_IsDevelopmentMode() )
        {
            for ( i = 0; i < 10; ++i )
            {
                if(cs->atest_par[i])
                    ent->m_seq_state->set(SeqBitNames(uint32_t(SeqBitNames::SB_TEST0)+i));
            }
        }
        */
    }
}

void checkFeet(Entity &/*e*/, SurfaceParams &/*surf_params*/)
{
    // TODO: implement geom collision check for feet.
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
}

// Move to Sequences or Triggers files later
void addTriggeredMove(Entity &e, TriggeredMove &trig)
{
    e.m_has_triggered_moves = true;
    e.m_triggered_moves.at(trig.m_move_idx) = trig;
}

//! @}
