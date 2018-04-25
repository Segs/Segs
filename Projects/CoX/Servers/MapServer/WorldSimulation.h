/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <ace/Time_Value.h>

#include "NetStructures/Entity.h"
#include "EntityStorage.h"
#include "SceneGraph.h"

class World
{
public:
                        World(EntityManager &em) : ref_ent_mager(em) {}
        void            addPlayer(Entity *ent);
        void            update(const ACE_Time_Value &tick_timer);
        float           time_of_day() const { return m_time_of_day; }
        float           sim_frame_time = 1; // in seconds
protected:
        void            physicsStep(Entity *e, uint32_t msec);
        void            effectsStep(Entity *e, uint32_t msec);
        void            updateEntity(Entity *e, const ACE_Time_Value &dT);
        EntityManager & ref_ent_mager;
        float           m_time_of_day = 8.0f; // hour of the day in 24h format, start at 8am
        ACE_Time_Value  prev_tick_time;
};
