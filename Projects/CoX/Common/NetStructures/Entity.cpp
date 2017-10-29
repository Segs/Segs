/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#define _USE_MATH_DEFINES
#include "Entity.h"

#include <ace/Log_Msg.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
//#define LOG_
//TODO: this file needs to know the MapInstance's WorldSimulation rate - Maybe extract it as a configuration object ?

#define WORLD_UPDATE_TICKS_PER_SECOND 30
static const float F_PI = float(M_PI); // to prevent double <-> float conversion warnings

float AngleDequantize(int value,int numb_bits)
{
    int max_val = 1<<numb_bits;
    float v = F_PI*(float(value)/max_val) - F_PI/2;
    if(v<(-F_PI/2))
        return -1.0f;
    else if(v>(F_PI/2))
        return 1.0f;
    else if(v<0.00001f)
        return 0.0f;
    return sinf(v);
}
int Entity::getOrientation(BitStream &bs)
{
    float fval;
    int update_qrot;
    update_qrot = getBitsConditional(bs,3);
    if(!update_qrot)
        return 0;
    bool recv_older = false;
    for(int i=0; i<3; i++)
    {
        if(update_qrot&(1<<i))
        {
            fval = AngleDequantize(bs.GetBits(9),9);
            if(current_client_packet_id>pkt_id_QrotUpdateVal[i])
            {
                pkt_id_QrotUpdateVal[i] = current_client_packet_id;
                qrot[i] = fval;
            }
            else
                recv_older=true;
        }
    }
    //RestoreFourthQuatComponent(pEnt->qrot);
    //NormalizeQuaternion(pEnt->qrot)
    return recv_older==false;
}

PlayerEntity::PlayerEntity()
{
    m_costume_type   = 1;
    m_change_existence_state         = true;
    m_destroyed      = false;
    m_type           = 2; // PLAYER
    m_create_player  = true;
    m_player_villain = false;
    m_origin_idx     = 0;
    m_class_idx      = 0;
    m_selector1      = false;
    m_hasname        = true;
    m_hasgroup_name  = false;
    m_pchar_things   = true;

    m_char.reset();
    might_have_rare = m_rare_bits = true;
}

void Entity::sendAllyID(BitStream &bs)
{
    bs.StorePackedBits(2,0);
    bs.StorePackedBits(4,0); // NPC->0

}
void Entity::sendPvP(BitStream &bs)
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
    bs.StorePackedBits(5,0);
    bs.StoreBits(1,0);
}

void Entity::fillFromCharacter(Character *f)
{
    m_char = *f;
    m_hasname = true;
    //TODO: map class/origin name to Entity's class/orign indices.
}
/**
 *  This will mark the Entity as being in logging out state
 *  \arg time_till_logout is time in seconds untill logout is done
 */
void Entity::beginLogout(uint16_t time_till_logout)
{
    m_is_logging_out = true;
    m_time_till_logout = time_till_logout*1000;
}

void PlayerEntity::serializefrom_newchar( BitStream &src,ColorAndPartPacker *packer )
{
    /*int val =*/ src.GetPackedBits(1); //2
    m_char.GetCharBuildInfo(src);
    m_char.recv_initial_costume(src,packer);
    /*int t =*/ src.GetBits(1); // The -> 1
    src.GetString(m_battle_cry);
    src.GetString(m_character_description);
}
void Entity::InsertUpdate( PosUpdate pup )
{
    m_update_idx++;
    m_update_idx %=64;
    m_pos_updates[m_update_idx]=pup;
}

void Entity::dump()
{
    m_char.dump();
}

void Entity::addPosUpdate(const PosUpdate & p) {
    m_update_idx = (m_update_idx+1) % 64;
    m_pos_updates[m_update_idx] = p;
}

void Entity::addInterp(const PosUpdate & p) {
    interpResults.emplace_back(p);
}

Entity::Entity()
{
    m_access_level = 9; // enables access to all deve
    pos = glm::vec3(-60.5f,180.0f,0.0f);
}

void sendPower(BitStream &bs,int a,int b,int c)
{
    bs.StorePackedBits(3,a);
    bs.StorePackedBits(3,b);
    bs.StorePackedBits(3,c);
}
void sendPowers(BitStream &bs)
{
    bs.StorePackedBits(4,0); // count
    for(int i=0; i<0; i++)
    {
        uint32_t num_powers=0;
        bs.StorePackedBits(5,0);
        bs.StorePackedBits(4,uint32_t(num_powers));
        for(uint32_t idx=0; idx<num_powers; ++idx)
        {
            uint32_t num_somethings=0;
            sendPower(bs,0,0,0);

            bs.StorePackedBits(5,0);
            bs.StoreFloat(1.0);
            bs.StorePackedBits(4,num_somethings);

            for(size_t idx2=0; idx2<num_somethings; ++idx2)
            {
                sendPower(bs,0,0,0);
                bs.StorePackedBits(5,0);
                bs.StorePackedBits(2,0);
            }
        }
    }
}
void sendPowers_main_tray(BitStream &bs)
{
    uint32_t max_num_cols=3;
    uint32_t max_num_rows=1;
    bs.StorePackedBits(3,max_num_cols); // count
    bs.StorePackedBits(3,max_num_rows); // count
    for(uint32_t i=0; i<max_num_cols; i++)
    {
        for(size_t idx=0; idx<max_num_rows; ++idx)
        {
            bool is_power=false;
            bs.StoreBits(1,is_power);
            if(is_power)
            {
                sendPower(bs,0,0,0);
            }
        }
    }
}

static void sendBoosts(BitStream &bs)
{
    uint32_t num_boosts=0;
    bs.StorePackedBits(5,num_boosts); // count
    for(size_t idx=0; idx<num_boosts; ++idx)
    {
        bool set_boost=false;
        bs.StorePackedBits(3,0); // bost idx
        bs.uStoreBits(1,set_boost); // 1 set, 0 clear
        if(set_boost)
        {
            sendPower(bs,0,0,0);
            bs.StorePackedBits(5,0); // bost idx
            bs.StorePackedBits(2,0); // bost idx
        }
    }
    // boosts
}
void sendUnk2(BitStream &bs)
{
    bs.StorePackedBits(5,0); // count
}
void sendUnk3(BitStream &bs) // inventory ?
{
    bs.StorePackedBits(3,0); // count
}

void abortLogout(Entity *e)
{
    e->m_logout_sent = false;
    e->m_is_logging_out = true; // send logout time of 0
    e->m_time_till_logout = 0;
}
