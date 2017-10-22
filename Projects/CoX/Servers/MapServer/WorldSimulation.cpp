#include "WorldSimulation.h"
#include "MapInstance.h"

#include <glm/gtx/vector_query.hpp>

void World::update(const ACE_Time_Value &tick_timer)
{
    ACE_Time_Value delta;
    if(prev_tick_time==ACE_Time_Value::zero) {
        delta = ACE_Time_Value(0,33*1000);
    }
    else
        delta = tick_timer - prev_tick_time;
    m_time_of_day+= 30*((float(delta.msec())/1000.0f)/(60.0*60)); // 1 sec of real time is 30s of ingame time
    if(m_time_of_day>=24.0f)
        m_time_of_day-=24.0f;
    sim_frame_time = delta.msec()/1000.0f;
    prev_tick_time = tick_timer;
    // TODO: use active entity list here
    for(int i=1; i<10240; ++i) {
        if(ref_ent_mager.m_map_entities[i]==nullptr)
            break;
        updateEntity(ref_ent_mager.m_map_entities[i],delta);
    }
}
void World::physicsStep(Entity *e,uint32_t msec)
{
    if(glm::length2(e->inp_state.pos_delta)) {
        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(e->inp_state.direction); // quat to mat4x4 conversion

        e->pos += ((za*e->inp_state.pos_delta)*float(msec))/50.0f;
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
    if(e->m_is_logging_out) {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}
