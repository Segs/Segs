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
    float traction = 0.0f;
    float friction = 0.0f;
    float bounce = 0.0f;
    float gravity = 0.0f;
    float max_speed = 0.0f;
};

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

void addPosUpdate(Entity &e, const PosUpdate &p);
void addInterp(const PosUpdate & p);
void forcePosition(Entity &e, glm::vec3 pos);

// Move to Sequences or Triggers files later
void addTriggeredMove(Entity &e, uint32_t move_idx, uint32_t delay, uint32_t fx_idx);
