#include "WorldSimulation.h"
#include "MapInstance.h"
#include "MapClient.h"
#include "AdminServer.h"
#include "Events/GameCommandList.h"
#include <glm/gtx/vector_query.hpp>

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
    prev_tick_time = tick_timer;
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_ent_mager.getEntitiesMutex());

    for(Entity * e : ref_ent_mager.m_live_entlist)
    {
        updateEntity(e,delta);
        if(e->m_client)
        {
            auto dmg = new FloatingDamage(e->m_idx,e->m_idx,1);
            e->m_client->addCommandToSendNextUpdate(std::unique_ptr<FloatingDamage>(dmg));
        }
    }
}
void World::physicsStep(Entity *e,uint32_t msec)
{
    if(glm::length2(e->inp_state.pos_delta))
    {
        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(e->inp_state.direction); // quat to mat4x4 conversion
        float vel_scale = e->inp_state.input_vel_scale/255.0f;
        e->pos += ((za*e->inp_state.pos_delta)*float(msec))/50.0f;
        e->vel = za*e->inp_state.pos_delta;
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
    
    if(e->m_type==Entity::ENT_PLAYER)
    {
//        CharacterDatabase *char_db = AdminServer::instance()->character_db();
//            char_db->update(&e->m_char);
    }
}
