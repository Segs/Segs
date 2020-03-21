/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <ace/Time_Value.h>

#include "MapServer.h"
#include "GameData/Entity.h"
#include "EntityStorage.h"
#include "SceneGraph.h"
#include "MapSceneGraph.h"

class World
{
public:
                        World(EntityManager &em, const float player_fade_in, MapInstance *owner_instance)
                            : ref_ent_mager(em), m_player_fade_in(player_fade_in), m_owner_instance(owner_instance) {}
        void            update(const ACE_Time_Value &tick_timer);
        float           time_of_day() const { return m_time_of_day; }
        float           sim_frame_time = 1; // in seconds
        float           accumulated_time=0;
protected:
        void            physicsStep(Entity *e, uint32_t msec);
        void            checkActivationTimers(Entity *e);
        void            checkDelayedEffects(Entity *e, uint32_t msec);
        void            checkRechargeTimers(Entity *e);
        void            checkAutoToggleTimers(Entity *e);
        void            checkBuffTimers(Entity *e);
        void            effectsStep(Entity *e, uint32_t msec);
        void            regenHealthEnd(Entity *e);
        void            updateEntity(Entity *e, const ACE_Time_Value &dT);
        void            collisionStep(Entity *e, uint32_t msec);
        EntityManager & ref_ent_mager;
        float           m_player_fade_in;
        float           m_time_of_day = 8.0f; // hour of the day in 24h format, start at 8am
        ACE_Time_Value  prev_tick_time;
        MapInstance     *m_owner_instance;
};
