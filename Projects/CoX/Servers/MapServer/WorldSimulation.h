#pragma once
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <ace/Time_Value.h>

#include "Entity.h"
#include "EntityStorage.h"

class World {
    float           m_time_of_day; // hour of the day in 24h format
public:
    World(EntityManager &em) : ref_ent_mager(em),m_time_of_day(8.0f) {} // start at 8am
    void addPlayer(Entity *ent) {
        ref_ent_mager.InsertPlayer(ent);
    }
    void update(const ACE_Time_Value &tick_timer) {
        ACE_Time_Value delta;
        if(prev_tick_time==ACE_Time_Value::zero) {
            delta = ACE_Time_Value(0,33*1000);
        }
        else
            delta = tick_timer - prev_tick_time;
        m_time_of_day+= 30*((float(delta.msec())/1000.0f)/(60.0*60)); // 1 sec of real time is 30s of ingame time
        if(m_time_of_day>=24.0f)
            m_time_of_day-=24.0f;
        timecount+=delta.msec();
        prev_tick_time = tick_timer;
        // TODO: use active entity list here
        for(int i=1; i<10240; ++i) {
            if(ref_ent_mager.m_map_entities[i]==nullptr)
                break;
            updateEntity(ref_ent_mager.m_map_entities[i],delta);
        }
    }
    float time_of_day() const {return m_time_of_day;}
    uint32_t timecount=1;
protected:
    void updateEntity(Entity *e,const ACE_Time_Value &dT);
    EntityManager &ref_ent_mager;
    ACE_Time_Value prev_tick_time;
};
