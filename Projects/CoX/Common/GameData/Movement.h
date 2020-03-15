/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "StateStorage.h"
#include <glm/glm.hpp>

class Entity;


struct PosUpdate // PosUpdatePair
{
    glm::vec3       m_position;
    glm::vec3       m_pyr_angles;
    uint32_t        m_timestamp;
    int             m_debug;
};

struct SurfaceParams
{
    float traction=0;
    float friction=0;
    float bounce=0;
    float gravitational_constant=0;
    float max_speed=0;
};
static_assert(2*sizeof(SurfaceParams)==320/8,"Required since it's sent as an bit array");

/*
 * Global Surface Params
 */
extern SurfaceParams g_world_surf_params[2];

enum CollFlags // TODO: Move to Collision file
{
    COLL_DISTFROMSTART      = 0x1,
    COLL_DISTFROMCENTER     = 0x2,
    COLL_HITANY             = 0x4,
    COLL_NEARESTVERTEX      = 0x8,
    COLL_NODECALLBACK       = 0x10,
    COLL_PORTAL             = 0x20,
    COLL_EDITONLY           = 0x40,
    COLL_BOTHSIDES          = 0x80,
    COLL_CYLINDER           = 0x100,
    COLL_ENTBLOCKER         = 0x200,
    COLL_PLAYERSELECT       = 0x400,
    COLL_NORMALTRI          = 0x800,
    COLL_FINDINSIDE         = 0x1000,
    COLL_FINDINSIDE_ANY     = 0x2000,
    COLL_TRICALLBACK        = 0x4000,
    COLL_GATHERTRIS         = 0x8000,
    COLL_DISTFROMSTARTEXACT = 0x10000,
    COLL_DENSITYREJECT      = 0x20000,
    COLL_IGNOREINVISIBLE    = 0x40000,
    COLL_NOTSELECTABLE      = 0x80000,
    COLL_SURF_SLICK         = 0x100000,
    COLL_SURF_ICY           = 0x200000,
    COLL_SURF_BOUNCY        = 0x400000,
    COLL_SURFACE_FLAGS      = 0x700000,
};

struct TriggeredMove // move to triggeredmove or sequences file later
{
    uint32_t    m_move_idx          = 0;
    uint32_t    m_ticks_to_delay    = 0;
    uint32_t    m_trigger_fx_idx    = 0;
};

enum StuckType
{
    STUCK_NONE          = 0x0,
    STUCK_SLIDE         = 0x1,
    STUCK_COMPLETELY    = 0x2,
};

enum MoveType
{
    MOVETYPE_WALK       = 0x1,
    MOVETYPE_FLY        = 0x2,
    MOVETYPE_NOCOLL     = 0x4,
    MOVETYPE_WIRE       = 0x8,
    MOVETYPE_JETPACK    = 0x10,
};

struct MotionState // current derived state of motion
{
    bool            m_is_falling            = false; // 0
    bool            m_is_stunned            = false; //
    bool            m_is_flying             = false; // 2
    bool            m_is_jumping            = false; // 3
    bool            m_is_bouncing           = false; // 6 , maybe repulsion? true when jumping
    bool            m_is_sliding            = false; // 8, 9, 10
    bool            m_has_jumppack          = false; //
    bool            m_has_headpain          = false; // formerly recover_from_landing_timer, but only used as bool?
    bool            m_is_slowed             = false; //
    bool            m_controls_disabled     = false; //
    bool            m_no_collision          = false; //
    uint8_t         m_motion_state_id       = 1;
    bool            m_update_motion_state   = true;     // EntityResponse sendServerControlState

    // Unknown Movement Flags
    bool            m_flag_1                = false; // 1 no idea
    bool            m_flag_5                = false; // 5 no idea, true when jumping
    bool            m_flag_12               = false; // 12 no idea, low friction?
    bool            m_flag_13               = false; // 13 no idea, increased traction?
    bool            m_flag_14               = false; // 14 no idea
    bool            m_flag_15               = false; // 15 no idea

    glm::vec3       m_velocity;
    glm::vec3       m_speed                 = {1,1,1};
    glm::vec3       m_input_velocity;
    glm::vec3       m_last_pos;

    StuckType       m_stuck                 = StuckType::STUCK_NONE;
    StuckType       m_stuck_head            = StuckType::STUCK_NONE;

    float           m_velocity_scale        = 0.0f;
    float           m_move_time             = 0.0f;
    float           m_backup_spd            = 1.0f;
    float           m_jump_height           = 2.0f;
    float           m_max_jump_height       = 0.0f;
    float           m_jump_apex             = 0.0f;
    float           m_jump_start            = 0.0f;
    int             m_jump_time             = 0;
    int             m_walk_flags            = 0;
    int             m_coll_surf_flags       = 0;
    int             m_field_88              = 0;

    glm::vec3       m_surf_normal;
    glm::vec3       m_surf_normal2;
    glm::vec3       m_surf_normal3;
    glm::vec3       m_surf_repulsion;

    SurfaceParams   m_surf_mods[2] = { {0,0,0,0,0},
                                     {0,0,0,0,0} };

    bool            m_debug = false; // wether to generate movement logging for this entity
};

void processNewInputs(Entity &e);

void addPosUpdate(Entity &e, const PosUpdate &p);
void forcePosition(Entity &e, glm::vec3 pos);
void forceOrientation(Entity &e, glm::vec3 pyr);

// Move to Sequences or Triggers files later
void addTriggeredMove(Entity &e, uint32_t move_idx, uint32_t delay, uint32_t fx_idx);
