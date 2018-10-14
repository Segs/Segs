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
    { 0.02f, 0.01f, 0.00f, 0.065f, 1.00f },  // air; from client
    //{ 1.00f, 0.45f, 0.01f, 0.065f, 1.00f }, // ground; test values
    //{ 0.02f, 0.01f, 0.00f, 0.065f, 1.00f },  // air; test values
};

void roundVelocityToZero(glm::vec3 *vel)
{
    if (std::abs(vel->x) < 0.00001f)
        vel->x = 0;
    if (std::abs(vel->y) < 0.00001f)
        vel->y = 0;
    if (std::abs(vel->z) < 0.00001f)
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
