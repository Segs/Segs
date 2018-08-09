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
#include "Logging.h"

#include <glm/gtx/vector_query.hpp>
#include <glm/ext.hpp>

SurfaceParams g_world_surf_params[2] = {
    // traction, friction, bounce, gravity, max_speed
    { 1.00f, 0.45f, 0.01f, 0.065f, 1.00f }, // ground; from client
    { 0.02f, 0.01f, 0.00f, 0.065f, 1.00f }  // air; from client
    //{ 1.50f, 1.50f, 1.50f, 3.000f, 1.50f }, // ground; from nem
    //{ 0.00f, 0.00f, 0.00f, 3.000f, 1.50f }  // air; from nem
};

void setVelocity(Entity &e)
{
    float       control_vals[6] = {0};
    glm::vec3   horiz_vel       = {0, 0, 0};
    float       max_press_time  = 0.0f;
    float       press_time      = 100.0f;
    glm::vec3   vel             = {0, 0, 0};

    if(e.m_states.current()->m_no_collision)
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "No collision" << e.m_states.current()->m_no_collision;

        e.m_velocity = vel;
    }

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

    e.m_states.current()->m_max_press_time = max_press_time;
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
            //ent->motion.flag_5 = false; // flag_5 "jogging anim"?
    }

    if(e.m_type == EntType::PLAYER)
        qCDebug(logMovement) << "horizVel:" << glm::to_string(horiz_vel).c_str();

    /* Movement Anims?
    if (optrel_speeds->fly)
        ent->move_type |= MOVETYPE_FLY;
    else
        ent->move_type &= ~MOVETYPE_FLY;
    if (optrel_speeds->flags_178_10)
        ent->move_type |= MOVETYPE_10;
    else
        ent->move_type &= ~MOVETYPE_10;
    */

    e.m_velocity = vel; // motion->inp_vel = vel;

    if (e.m_char->m_char_data.m_afk && e.m_velocity != glm::vec3(0,0,0))
    {
        if(e.m_type == EntType::PLAYER)
            qCDebug(logMovement) << "Moving so turning off AFK";

        toggleAFK(*e.m_char);
    }
}

void addPosUpdate(Entity &e, const PosUpdate &p)
{
    e.m_update_idx = (e.m_update_idx+1) % 64;
    e.m_pos_updates[e.m_update_idx] = p;
}

void addInterp(Entity &e, const PosUpdate &p)
{
    e.m_interp_results.emplace_back(p);
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

//! @}
