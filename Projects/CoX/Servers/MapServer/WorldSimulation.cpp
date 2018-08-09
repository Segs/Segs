/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "WorldSimulation.h"
#include "MapInstance.h"

#include "Common/Servers/Database.h"
#include "Common/NetStructures/Movement.h"
#include "Events/GameCommandList.h"

#include <glm/gtx/vector_query.hpp>
#include <glm/ext.hpp>

void World::update(const ACE_Time_Value &tick_timer)
{

    ACE_Time_Value delta;
    if(prev_tick_time==ACE_Time_Value::zero)
    {
        delta = ACE_Time_Value(0,33*1000);
    }
    else
        delta = tick_timer - prev_tick_time;
    m_time_of_day+= 4.8*((float(delta.msec())/1000.0f)/(60.0*60)); // 1 sec of real time is 48s of ingame time
    if(m_time_of_day>=24.0f)
        m_time_of_day-=24.0f;
    sim_frame_time = delta.msec()/1000.0f;
    accumulated_time += sim_frame_time;
    prev_tick_time = tick_timer;
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_ent_mager.getEntitiesMutex());

    for(Entity * e : ref_ent_mager.m_live_entlist)
    {
        updateEntity(e,delta);
    }
}

void World::physicsStep(Entity *e,uint32_t msec)
{
    if(e->m_cur_state == nullptr)
        return;

    if(glm::length2(e->m_cur_state->m_pos_delta))
    {
        SetVelocity(e);

        // Get timestamp in ms
        auto now_ms = std::chrono::steady_clock::now().time_since_epoch().count();

        // add PosUpdate
        PosUpdate prev      = e->m_pos_updates[(e->m_update_idx + -1 + 64) % 64];
        PosUpdate pud;
        pud.m_position      = e->m_entity_data.m_pos;
        pud.m_pyr_angles    = e->m_entity_data.m_orientation_pyr;
        pud.m_timestamp     = now_ms;
        e->addPosUpdate(pud);

        int dt = pud.m_timestamp - prev.m_timestamp;
        e->m_prev_pos = e->m_entity_data.m_pos;

        glm::mat3 za = static_cast<glm::mat3>(e->m_direction); // quat to mat4x4 conversion
        //e->m_entity_data.m_pos += ((za*e->m_cur_state->m_pos_delta)*float(msec))/e->u1; // logmovement: why 50? why?
        e->m_entity_data.m_pos += ((za*e->m_cur_state->m_pos_delta)*float(msec))/50;

        if(e->m_type == EntType::PLAYER)
        {
            float vel_scale = e->m_cur_state->m_velocity_scale/255.0f;
            float distance  = glm::distance(e->m_entity_data.m_pos, e->m_prev_pos);
            qCDebug(logMovement) << "physicsStep:"
                                       << "\n    prev_pos:\t"   << glm::to_string(e->m_prev_pos).c_str()
                                       << "\n    cur_pos:\t"    << glm::to_string(e->m_entity_data.m_pos).c_str()
                                       << "\n    distance:\t"   << distance
                                       << "\n    vel_scale:\t"  << vel_scale << e->m_cur_state->m_velocity_scale
                                       << "\n    velocity:\t"   << glm::to_string(e->m_velocity).c_str();
        }
    }
}

float animateValue(float v,float start,float target,float length,float dT)
{
    float range=target-start;
    float current_pos = (v-start)/range;
    float accumulated_time = length*current_pos;
    accumulated_time = std::min(length,accumulated_time+dT);
    float res = start + (accumulated_time/length) * range;
    return res;
}

void World::effectsStep(Entity *e,uint32_t msec)
{
    if(e->m_is_fading)
    {
        float target=0.0f;
        float start=1.0f;
        if(e->m_fading_direction!=FadeDirection::In)
        { // must be fading out, so our target is 100% transparency.
            target = 1;
            start = 0;
        }
        e->translucency = animateValue(e->translucency,start,target,m_player_fade_in,float(msec)/50.0f);
        if(std::abs(e->translucency-target)<std::numeric_limits<float>::epsilon())
            e->m_is_fading = false;
    }
}

void World::updateEntity(Entity *e, const ACE_Time_Value &dT) {
    physicsStep(e,dT.msec());
    effectsStep(e,dT.msec());
    if(e->m_is_logging_out)
    {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}

//! @}
