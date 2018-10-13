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

#include "Common/Servers/Database.h"
#include "Events/GameCommandList.h"
#include "NetStructures/Character.h"
#include "NetStructures/CharacterHelpers.h"
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
    m_time_of_day+= 4.8f*((float(delta.msec())/1000.0f)/(60.0f*60)); // 1 sec of real time is 48s of ingame time
    if(m_time_of_day>=24.0f)
        m_time_of_day-=24.0f;
    sim_frame_time = delta.msec()/1000.0f;
    accumulated_time += sim_frame_time;
    prev_tick_time = tick_timer;
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_ent_mager.getEntitiesMutex());

    for(Entity * e : ref_ent_mager.m_live_entlist)
        updateEntity(e,delta);
}

void World::physicsStep(Entity *e,uint32_t msec)
{
    if(glm::length2(e->m_states.current()->m_pos_delta))
    {
        // todo: take into account time between updates
        glm::mat3 za = static_cast<glm::mat3>(e->m_direction); // quat to mat4x4 conversion
        //float vel_scale = e->inp_state.m_input_vel_scale/255.0f;
        e->m_entity_data.m_pos += ((za*e->m_states.current()->m_pos_delta)*float(msec))/50.0f;
        e->m_motion_state.m_velocity = za*e->m_states.current()->m_pos_delta;
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

void World::checkPowerTimers(Entity *e, uint32_t msec)
{
    // for now we only run this on players
    if(e->m_type != EntType::PLAYER)
        return;

    // Activation Timers -- queue FIFO
    if(e->m_queued_powers.size() > 0)
    {
        QueuedPowers &qpow = e->m_queued_powers.front();
        qpow.m_activate_period -= (float(msec)/1000);

        // this must come before the activation_period check
        // to ensure that the queued power ui-effect is reset properly
        if(qpow.m_activation_state == false)
        {
            e->m_queued_powers.dequeue(); // remove first from queue
            e->m_char->m_char_data.m_has_updated_powers = true;
        }

        if(qpow.m_activate_period <= 0)
            qpow.m_activation_state = false;
    }

    // Recharging Timers -- iterate through and remove finished timers
    auto rpow_idx = e->m_recharging_powers.begin();
    for(QueuedPowers &rpow : e->m_recharging_powers)
    {
        rpow.m_recharge_time -= (float(msec)/1000);

        if(rpow.m_recharge_time <= 0)
        {
            rpow_idx = e->m_recharging_powers.erase(rpow_idx);

            // Check if rpow is default power, if so usePower again
            if(e->m_char->m_char_data.m_trays.m_has_default_power)
            {
                if(rpow.m_pow_idxs.m_pset_vec_idx == e->m_char->m_char_data.m_trays.m_default_pset_idx
                        && rpow.m_pow_idxs.m_pow_vec_idx == e->m_char->m_char_data.m_trays.m_default_pow_idx)
                {
                    usePower(*e, rpow.m_pow_idxs.m_pset_vec_idx, rpow.m_pow_idxs.m_pow_vec_idx, getTargetIdx(*e), getTargetIdx(*e));
                }
            }

            e->m_char->m_char_data.m_has_updated_powers = true;
        }
        else
            ++rpow_idx;
    }

    // Buffs
    auto buff_idx = e->m_buffs.begin();
    for(Buffs &buff : e->m_buffs)
    {
        buff.m_activate_period -= (float(msec)/1000); // activate period is in minutes

        if(buff.m_activate_period <= 0)
            buff_idx = e->m_buffs.erase(buff_idx);
        else
            ++buff_idx;
    }
}

bool World::isPlayerDead(Entity *e)
{
    if(e->m_type == EntType::PLAYER
            && getHP(*e->m_char) == 0.0)
    {
        setStateMode(*e, ClientStates::DEAD);
        return true;
    }

    return false;
}

void World::regenHealthEnd(Entity *e, uint32_t msec)
{
    // for now on Players only
    if(e->m_type == EntType::PLAYER)
    {
        float hp = getHP(*e->m_char);
        float end = getEnd(*e->m_char);

        float regeneration = hp * (1.0/20.0) * msec/1000/12;
        float recovery = end * (1.0/15.0) * msec/1000/12;
        setHP(*e->m_char, hp + regeneration);
        setEnd(*e->m_char, end + recovery);
    }
}

void World::updateEntity(Entity *e, const ACE_Time_Value &dT)
{
    physicsStep(e, dT.msec());
    effectsStep(e, dT.msec());
    checkPowerTimers(e, dT.msec());

    // check death, set clienstate if dead, and
    // if alive, recover endurance and health
    if(!isPlayerDead(e))
        regenHealthEnd(e, dT.msec());

    if(e->m_is_logging_out)
    {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
}

//! @}
