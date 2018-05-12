/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "WorldSimulation.h"

#include "MapInstance.h"

#include "Common/Servers/Database.h"
#include "Events/GameCommandList.h"
#include <glm/gtx/vector_query.hpp>

void markFlying(Entity &e ,bool is_flying) // Function to set character as flying
{

    e.m_is_flying = is_flying;

}

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

        if ((int)tick_timer.sec() % m_char_update_interval == 0) // update the player characters in DB every n seconds
        {
            if (e->hasValidDbStoreFlag())
                charUpdateDB(e);
        }
    }
}

void World::physicsStep(Entity *e,uint32_t msec)
{
    if(glm::length2(e->inp_state.pos_delta))
    {
        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(e->m_direction); // quat to mat4x4 conversion
        //float vel_scale = e->inp_state.m_input_vel_scale/255.0f;
        e->m_entity_data.m_pos += ((za*e->inp_state.pos_delta)*float(msec))/50.0f;
        e->m_velocity = za*e->inp_state.pos_delta;
    }

//    if(e->inp_state.pos_delta[1] == 1.0f) // Will set 'is flying' on jump event
//        markFlying(*e, true);
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
        e->translucency = animateValue(e->translucency,start,target,380.0f,float(msec)/50.0f);
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

    /*
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    // TODO: Implement asynchronous database queries
    DbTransactionGuard grd(*char_db->getDb());
    if(false==char_db->update(e))
        return;
    grd.commit();
    */
}

void World::addPlayer(Entity *ent)
{
    ref_ent_mager.InsertPlayer(ent);
}

//! @}
