/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include <glm/glm.hpp>

class Entity;

struct PosUpdate // PosUpdatePair
{
    glm::vec3       m_position;
    glm::vec3       m_pyr_angles;
    int             m_timestamp;
    int             m_debug;
};

struct SurfaceParams
{
    float traction;
    float friction;
    float bounce;
    float gravitational_constant;
    float max_speed;
};

extern SurfaceParams g_world_surf_params[2];

enum StuckType
{
    STUCK_NONE          = 0x0,
    STUCK_SLIDE         = 0x1,
    STUCK_COMPLETELY    = 0x2,
};

struct MotionState
{
    bool            m_is_falling            = false; // 0
    bool            m_is_stunned            = false; //
    bool            m_is_flying             = false; // 2
    bool            m_is_jumping            = false; // 3
    bool            m_is_sliding            = false; // 8, 9, 10
    bool            m_has_jumppack          = false; //
    bool            m_controls_disabled     = false; //
    bool            m_no_collision          = false; //
    uint8_t         m_motion_state_id       = 1;
    bool            m_update_motion_state   = true;     // EntityResponse sendServerControlState

    StuckType       m_stuck             = STUCK_NONE;
    StuckType       m_stuck_head        = STUCK_NONE;

    float           m_backup_spd            = 1.0f;
    float           m_jump_height           = 2.0f;
    float           m_max_jump_height   = 0;
    float           m_highest_height    = 0;
    int             m_jump_time         = 0;
    int             m_walk_flags        = 0;

    SurfaceParams   surf_mods[2] = { {0,0,0,0,0},
                                     {0,0,0,0,0} };
};

void setVelocity(Entity &e);
void addPosUpdate(Entity &e, const PosUpdate &p);
void addInterp(Entity &e, const PosUpdate &p);
bool updateRotation(const Entity &e, int axis); // returns true if given axis needs updating;
void forcePosition(Entity &e, glm::vec3 pos);
