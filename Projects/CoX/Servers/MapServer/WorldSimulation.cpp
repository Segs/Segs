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
#include <glm/ext.hpp>

static void SetVelocity(Entity *e)
{
    float control_vals[6] = {0};
    glm::vec3 horiz_vel = {0, 0, 0};
    int max_press_time = 0;
    int press_time = 100;
    glm::vec3 vel = {0, 0, 0};

    if(e->m_cur_state->m_no_collision)
    {
        qCDebug(logMovement) << "No collision" << e->m_cur_state->m_no_collision;
        e->m_velocity = vel;
    }

    for (int i = BinaryControl::FORWARD; i < BinaryControl::LAST_BINARY_VALUE; ++i)
    {
        press_time = e->m_cur_state->m_keypress_time[i]*30;
        //qCDebug(logMovement) << "keypress_time" << i << e->inp_state.m_keypress_time[i];
        max_press_time = std::max(press_time, max_press_time);
        if (i >= BinaryControl::UP && !e->m_is_flying) // UP or Fly
            control_vals[i] = (float)(press_time != 0);
        else if (!press_time)
            control_vals[i] = 0.0f;
        else if (press_time >= 1000)
            control_vals[i] = 1.0f;
        else if (press_time <= 50 /*&& e->inp_state.m_control_bits[i]*/)
            control_vals[i] = 0.0f;
        else if (press_time < 75)
            control_vals[i] = 0.2f;
        else if (press_time < 100)
            control_vals[i] = std::pow(float(press_time - 75) * 0.04f, 2.0f) * 0.4f + 0.2f;
        else
            control_vals[i] = (float)(press_time - 100) * 0.004f / 9.0f + 0.6f;

        //qCDebug(logMovement) << "control_vals:" << i << control_vals[i];
    }

    //controls->max_presstime = max_press_time;
    vel.x = control_vals[BinaryControl::RIGHT] - control_vals[BinaryControl::LEFT];
    vel.y = control_vals[BinaryControl::UP] - control_vals[BinaryControl::DOWN];
    vel.z = control_vals[BinaryControl::FORWARD] - control_vals[BinaryControl::BACKWARD];
    vel.x = vel.x * e->m_speed.x;
    vel.y = vel.y * e->m_speed.y;

    qCDebug(logMovement) << "vel:" << glm::to_string(vel).c_str();

    horiz_vel = vel;

    if (!e->m_is_flying)
        horiz_vel.y = 0;
    if (vel.z < 0.0f)
        e->m_cur_state->m_velocity_scale *= e->m_backup_spd;
    if (e->m_is_stunned)
        e->m_cur_state->m_velocity_scale *= 0.1f;

    if(horiz_vel.length() >= 0.0f)
        horiz_vel = glm::normalize(horiz_vel);

    if (e->m_cur_state->m_speed_scale != 0.0f)
        e->m_cur_state->m_velocity_scale *= e->m_cur_state->m_speed_scale;

    vel.x = horiz_vel.x * std::fabs(control_vals[BinaryControl::RIGHT] - control_vals[BinaryControl::LEFT]);
    vel.z = horiz_vel.z * std::fabs(control_vals[BinaryControl::FORWARD] - control_vals[BinaryControl::BACKWARD]);

    if (e->m_is_flying)
        vel.y = horiz_vel.y * std::fabs(control_vals[BinaryControl::UP] - control_vals[BinaryControl::DOWN]);
    else if (e->m_cur_state->m_prev_control_bits[BinaryControl::UP])
        vel.y = 0;
    else
    {
        vel.y *= glm::clamp(e->m_jump_height, 0.0f, 1.0f);
        //if (!e->m_is_sliding)
            //ent->motion.flag_5 = false;
    }

    qCDebug(logMovement) << "horizVel:" << glm::to_string(horiz_vel).c_str();

    e->m_velocity = vel;

    if (e->m_char->m_char_data.m_afk && e->m_velocity != glm::vec3(0,0,0))
    {
        qCDebug(logMovement) << "Moving so turning off AFK";
        toggleAFK(*e->m_char);
    }
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
    }
}

void World::physicsStep(Entity *e,uint32_t msec)
{
    if(e->m_cur_state == nullptr)
        return;

    if(glm::length2(e->m_cur_state->pos_delta))
    {
        SetVelocity(e);

        PosUpdate prev = e->m_pos_updates[(e->m_update_idx + -1 + 64) % 64];
        PosUpdate current;
        current.m_position     = e->m_entity_data.m_pos;
        current.m_pyr_angles   = e->m_entity_data.m_orientation_pyr;
        current.m_timestamp    = msec;
        e->addPosUpdate(current);

        int dt = current.m_timestamp - prev.m_timestamp;
        e->m_prev_pos = e->m_entity_data.m_pos;

        glm::mat3 za = static_cast<glm::mat3>(e->m_direction); // quat to mat4x4 conversion
        float vel_scale = e->m_cur_state->m_velocity_scale/255.0f;

        e->m_entity_data.m_pos += ((za*e->m_cur_state->pos_delta)*float(msec))/50.0f;
        float distance  = glm::distance(e->m_entity_data.m_pos, e->m_prev_pos);
        //e->m_velocity   = e->inp_state.pos_delta * e->m_speed / distance;
        //e->m_velocity   = za*e->inp_state.pos_delta;

        qCDebug(logMovement) << "physicsStep:"
                             << "\n    prev_pos:\t"   << glm::to_string(e->m_prev_pos).c_str()
                             << "\n    cur_pos:\t"    << glm::to_string(e->m_entity_data.m_pos).c_str()
                             << "\n    distance:\t"   << distance
                             << "\n    vel_scale:\t"  << vel_scale << e->m_cur_state->m_velocity_scale
                             << "\n    velocity:\t"   << glm::to_string(e->m_velocity).c_str();
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
