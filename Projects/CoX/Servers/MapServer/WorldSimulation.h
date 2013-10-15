#pragma once
#include <osg/Matrix>
#include <osg/Vec3>
#include "Entity.h"
#include "EntityStorage.h"

class World {
public:
    World(EntityManager &em) : ref_ent_mager(em) {}
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
        timecount+=delta.msec();
        prev_tick_time = tick_timer;
        // TODO: use active entity list here
        for(int i=1; i<10240; ++i) {
            if(ref_ent_mager.m_map_entities[i]==nullptr)
                break;
            updateEntity(ref_ent_mager.m_map_entities[i],delta);
        }
    }
    uint32_t timecount=1;
protected:
    void updateEntity(Entity *e,const ACE_Time_Value &dT) {
        if(e->inp_state.pos_delta.length2()) {
            // todo: take into account time between updates
            osg::Matrix za(osg::Matrix::rotate(e->inp_state.direction));

            e->pos += ((za*e->inp_state.pos_delta)*dT.msec())/50.0f;
        }
    }
    EntityManager &ref_ent_mager;
    ACE_Time_Value prev_tick_time;
};
