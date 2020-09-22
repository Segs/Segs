/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "WorldSimulation.h"

#include "DataHelpers.h"
#include "NetFxHelpers.h"
#include "MapInstance.h"

#include "Common/Messages/Map/FloatingInfoStyles.h"
#include "Common/Servers/Database.h"
#include "Common/Servers/InternalEvents.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "Runtime/Systems/FXSystem.h"
//#include "Messages/Map/GameCommandList.h"

#include <glm/gtx/vector_query.hpp>

using namespace SEGSEvents;

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

    updateNetFx(); // cleans up the global netfx storage
    for(Entity * e : ref_ent_mager.m_live_entlist)
    {
        updateEntity(e,delta);
        if(e->m_destroyed)
            break;
    }
    FXSystem::update(sim_frame_time);
}

void World::physicsStep(Entity *e,uint32_t msec)
{
    processNewInputs(*e);
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
        {
            if (e->m_char->m_is_dead)
                m_owner_instance->m_entities.removeEntityFromActiveList(e);
            else
                e->m_is_fading = false;
        }
    }
}

// Delayed and repeated effects are queued on the target, and activate an effect when the timer counts down
void World::checkDelayedEffects(Entity *e, uint32_t msec)
{
    if(e->m_type == EntType::CRITTER)
    {
        if (rand() % 300 == 0)
        {
            if (e->m_aggro_list.size() > 0)
            {
                QString msg = "say I'm harmless!";
                if (e->m_npc->src_data->m_Powers.size() > 0)
                {
                Entity * tgt = getEntity(m_owner_instance, e->m_aggro_list[0].idx);

                if (tgt == nullptr || tgt->m_char->m_is_dead || glm::distance(tgt->m_entity_data.m_pos, e->m_entity_data.m_pos) > 100)
                {
                    e->m_aggro_list[0].aggro = 0;
                    qCDebug(logNPCs) << e->m_aggro_list[0].name << "moved to front of aggrolist";
                }
                if (e->m_aggro_list[0].aggro > 0)
                {
                    msg = " " + e->m_aggro_list[0].name + " has " + QString("%1 aggro") .arg(e->m_aggro_list[0].aggro);
                }
                else
                {
                    msg = "say I don't care about "+ e->m_aggro_list[0].name +" anymore.";

                    if (e->m_aggro_list.size() == 1 || e->m_aggro_list[1].aggro <= 0)       //this could be sent to the EM so it knows combat is over
                        e->m_aggro_list.clear();            //no more targets with aggro so drop
                    else
                    {
                        Aggro swap = e->m_aggro_list[0];
                        e->m_aggro_list.pop_front();
                        qCDebug(logNPCs) << e->m_aggro_list[0].name << "now has aggro";
                        e->m_aggro_list.push_back(swap);    //this isn't sorted, but good enough to find a new target
                    }

                }
                m_owner_instance->add_chat_message(e, msg);

                }//don't msg if no powers
            }
            else
            {
                for (auto _near : m_owner_instance->m_entities.m_live_entlist)
                    if (_near != nullptr && !_near->m_char->m_is_dead &&  validTarget(*_near, *e, StoredEntEnum::Foe)
                            && glm::distance(_near->m_entity_data.m_pos, e->m_entity_data.m_pos) < 50)
                    {
                        //this could be sent to the EM to send to all critteres in this spawn
                        QString msg = " I see " + _near->name();
                        m_owner_instance->add_chat_message(e, msg);

                        e->m_aggro_list.push_back({ _near->name(),_near->m_idx,1,0});// 1 point of aggro
                    }
            }
        }
    }

    for(auto dly_idx = e->m_delayed.begin(); dly_idx != e->m_delayed.end(); /*dly_idx updated inside loop*/ )
    {
        dly_idx->m_timer -= msec;
        if(dly_idx->m_timer < 0)            // if delay is 0 this will happen right away
        {
            Entity *tgt = getEntity(e, m_owner_instance, dly_idx->src_ent);
            doAtrrib( *tgt, e, dly_idx->mod, dly_idx->power);

            if (dly_idx->ticks >1)
            {
                dly_idx->m_timer = int(dly_idx->mod.Duration*1000/dly_idx->mod.Period);
                dly_idx->ticks--;
            }
            else
                dly_idx->ticks = -1;
        }

        if(dly_idx->ticks < 0)
            dly_idx = e->m_delayed.erase(dly_idx);
        else
            ++dly_idx;
    }
}

// Activated powers are checked to make sure they have valid contions, and then count down timer to activate power -- queue FIFO
// Also "default powers" are queued up if one has been set, and nothing else is queued for activation
void World::checkActivationTimers(Entity *e)
{
    if(e->m_queued_powers.size() > 0)
    {
        QueuedPowers &qpow = e->m_queued_powers.front();
        if (qpow.m_activation_state)           //sometimes powers get turned off before they can be removed from the queue
        {
            if (!e->m_is_activating)
            {
                if (checkPowerBlock(*e))
                {
                    qpow.m_activation_state = false;                //this will turn off the activation ring and then dequeue
                    qpow.m_active_state_change = true;
                    e->m_char->m_char_data.m_has_updated_powers = true;
                }
                else
                {
                    Entity *target_ent = getEntity(e,m_owner_instance,qpow.m_tgt_idx);
                    CharacterPower * ppower =  getOwnedPowerByVecIdx(*e, qpow.m_pow_idxs.m_pset_vec_idx, qpow.m_pow_idxs.m_pow_vec_idx);

                    if (checkPowerRecharge(*e, qpow.m_pow_idxs.m_pset_vec_idx, qpow.m_pow_idxs.m_pow_vec_idx)
                        && checkPowerRange(*e, *target_ent, ppower->getPowerTemplate().Range))
                    {
                        e->m_is_activating = true;       //queued power can move forward to an active power
                        checkMovement(*e);               //stop movement while casting
                        e->m_char->m_char_data.m_has_updated_powers = true;
                    }
                }
            }
            else
            {
                qpow.m_time_to_activate -= sim_frame_time;

                if(qpow.m_time_to_activate < 0 && qpow.m_active_state_change == false)
                {
                    CharacterPower * ppower = getOwnedPowerByVecIdx(*e, qpow.m_pow_idxs.m_pset_vec_idx, qpow.m_pow_idxs.m_pow_vec_idx);
                    if (ppower->getPowerTemplate().Type == PowerType::Toggle)
                    {
                        e->m_auto_powers.push_back(qpow);
                        e->m_queued_powers.pop_front();
                    }
                    else
                    {
                        doPower(*e, qpow);
                        qpow.m_activation_state = false;                //this will turn off the activation ring and then dequeue
                        qpow.m_active_state_change = true;
                        e->m_char->m_char_data.m_has_updated_powers = true;
                    }
                    e->m_is_activating = false;
                    checkMovement(*e);                                  //frees up movement, unless held by other means
                }
                else
                {
                    QString from_msg = "charging!";                     //so players know they are activating a power
                    sendFloatingInfo(*e->m_client, from_msg, FloatingInfoStyle::FloatingInfo_Info, 0.0);
                }
            }
        }
    }
    else
    {           // only activate the default power if there is no other power in the activation queue
        PowerTrayGroup &trays =  e->m_char->m_char_data.m_trays;
        if(trays.m_has_default_power)
        {
            if (checkPowerRecharge(*e, trays.m_default_pset_idx, trays.m_default_pow_idx))
            {
                usePower(*e, trays.m_default_pset_idx, trays.m_default_pow_idx, getTargetIdx(*e));
            }
        }
    }
}

// Power that have been used are placed in this queue, when the timer counts down they are removed and can be used again
void World::checkRechargeTimers(Entity *e)
{
    for(auto rpow_idx = e->m_recharging_powers.begin(); rpow_idx != e->m_recharging_powers.end(); /*rpow_idx updated inside loop*/ )
    {
        rpow_idx->m_recharge_time -= sim_frame_time;

        if(rpow_idx->m_recharge_time <= 0)
        {
            rpow_idx = e->m_recharging_powers.erase(rpow_idx);
            e->m_char->m_char_data.m_has_updated_powers = true;
        }
        else
            ++rpow_idx;
    }
}

// Toggles and powers that turn automatically fire an effect every time the timer counts down, toggles have condtions when they are forced off
void World::checkAutoToggleTimers(Entity *e)
{
    for(auto rpow_idx = e->m_auto_powers.begin(); rpow_idx != e->m_auto_powers.end();)
    {
        CharacterPower * ppower = getOwnedPowerByVecIdx(*e, rpow_idx->m_pow_idxs.m_pset_vec_idx, rpow_idx->m_pow_idxs.m_pow_vec_idx);
        const Power_Data powtpl = ppower->getPowerTemplate();

        if(rpow_idx->m_time_to_activate   <= 0)
        {
            doPower(*e, *rpow_idx);
            rpow_idx->m_time_to_activate += rpow_idx->m_activate_period;
        }
        else
        {
            rpow_idx->m_time_to_activate   -= sim_frame_time;
        }
        if (powtpl.Type == PowerType::Toggle)
        {
                Entity *target_ent = getEntity(e, e->m_client->m_current_map, rpow_idx->m_tgt_idx);
                CharacterPower * ppower =  getOwnedPowerByVecIdx(*e, rpow_idx->m_pow_idxs.m_pset_vec_idx, rpow_idx->m_pow_idxs.m_pow_vec_idx);

                if((getEnd(*e->m_char) < powtpl.EnduranceCost)  || (e->m_char->m_is_dead) || (target_ent->m_char->m_is_dead)
                    || !checkPowerRange(*e, *target_ent, ppower->getPowerTemplate().Range))

                {
                    rpow_idx->m_activation_state = false;
                }
        }
        if ( rpow_idx->m_activation_state == false)
        {
            rpow_idx->m_active_state_change = true;
            e->m_queued_powers.push_back(*rpow_idx);            //this sends the deactivation state to the client
            e->m_recharging_powers.push_back(*rpow_idx);
            e->m_auto_powers.erase(rpow_idx);
            e->m_char->m_char_data.m_has_updated_powers = true;
        }
        else
        {
            ++rpow_idx;
        }
    }
}

// Buffs are placed in a queue on the target entity, when the timer runs out the stat changes are reversed
void World::checkBuffTimers(Entity *e)
{
    for(auto thisbuff = e->m_buffs.begin(); thisbuff != e->m_buffs.end(); /*thisbuff updated inside loop*/)
    {
        for (auto bff = thisbuff->m_buffs.begin(); bff != thisbuff->m_buffs.end(); /*bff updated inside loop*/)
            if(bff->m_duration <= 0 || e->m_char->m_is_dead)
            {
                bff->m_value = -bff->m_value;
                modifyAttrib(*e, *bff);
                bff = thisbuff->m_buffs.erase(bff);
            }
            else
            {
                bff->m_duration -= sim_frame_time;                 // duration is in seconds
                ++bff;
            }

        if (thisbuff->m_buffs.empty())
            thisbuff = e->m_buffs.erase(thisbuff);
        else
            ++thisbuff;
    }
}

void World::regenHealthEnd(Entity *e)
{
    float hp = getHP(*e->m_char);
    float end = getEnd(*e->m_char);

    float regeneration = getMaxHP(*e->m_char) * (e->m_char->m_char_data.m_current_attribs.m_Regeneration/20.0f * sim_frame_time/12);
    float recovery = getMaxEnd(*e->m_char) * (e->m_char->m_char_data.m_current_attribs.m_Recovery/4.9f * sim_frame_time/12);

    if(hp < getMaxHP(*e->m_char))
        setHP(*e->m_char, hp + regeneration);
    if(end < getMaxEnd(*e->m_char))
        setEnd(*e->m_char, end + recovery);
}

void World::collisionStep(Entity *e, uint32_t /*msec*/)
{
    if (e->m_player != nullptr && !e->m_map_swap_collided)
    {
        // Range-For only uses the values, so you can't get the keys unless you use toStdMap() or iterate keys().
        // Both are less efficient than just using an iterator.
        QHash<QString, MapXferData>::const_iterator i = m_owner_instance->get_map_zone_transfers().constBegin();
        while (i != m_owner_instance->get_map_zone_transfers().constEnd())
        {
            // TODO: This needs to check against the trigger plane for transfers. This should be part of the wall objects geobin. Also need to make sure that this doesn't cause players to immediately zone after being spawned in a spawnLocation near a zoneline.            
            if ((e->m_entity_data.m_pos.x >= i.value().m_position.x - 20 && e->m_entity_data.m_pos.x <= i.value().m_position.x + 20) &&
                (e->m_entity_data.m_pos.y >= i.value().m_position.y - 20 && e->m_entity_data.m_pos.y <= i.value().m_position.y + 20) &&
                (e->m_entity_data.m_pos.z >= i.value().m_position.z - 20 && e->m_entity_data.m_pos.z <= i.value().m_position.z + 20))
            {
                e->m_map_swap_collided = true;  // So we don't send repeated events for the same entity
                m_owner_instance->putq(new MapSwapCollisionMessage({e->m_db_id, e->m_entity_data.m_pos, i.key()}, 0));
                return; // don't want to keep checking for other maps for this entity
            }
            i++;

        }
    }
}

void World::updateEntity(Entity *e, const ACE_Time_Value &dT)
{
    physicsStep(e, uint32_t(dT.msec()));
    effectsStep(e, uint32_t(dT.msec()));

    // for now we only run these on players
    if(e->m_type == EntType::PLAYER)
    {
        checkActivationTimers(e);
        checkRechargeTimers(e);
        checkAutoToggleTimers(e);
    }

    checkBuffTimers(e);
    checkDelayedEffects(e, uint32_t(dT.msec()));
    collisionStep(e, uint32_t(dT.msec()));
    // TODO: Issue #555 needs to handle team cleanup properly
    // and we need to remove the following
    if(e->m_team != nullptr)
    {
        if(e->m_team->m_data.m_team_members.size() <= 1)
        {
            qWarning() << "Team cleanup being handled in updateEntity, but we need to move this to TeamHandler";
            e->m_has_team = false;
            e->m_team = nullptr;
        }
    }

    // check death, set clienstate if dead, and
    // if alive, recover endurance and health
    if(e->m_char != nullptr)                            //either a player or critter
    {
        if (!e->m_char->m_is_dead)
            regenHealthEnd(e);
        else if (e->m_type == EntType::CRITTER && !e->m_is_fading)// fading state is set after rewards, so it is only rewarded once here
            grantRewards(m_owner_instance->m_entities, *e);
    }
    if(e->m_is_logging_out)
    {
        e->m_time_till_logout -= dT.msec();
        if(e->m_time_till_logout<0)
            e->m_time_till_logout=0;
    }
    for(auto iter=e->m_net_fx.begin(); iter!=e->m_net_fx.end(); ++iter)
    {
        auto handle = *iter;
        if(updateNetFxFromParent(handle))
        {
            iter = e->m_net_fx.erase(iter);
        }
    }
}

//! @}
