/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#define _USE_MATH_DEFINES
#include "Entity.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
//#define LOG_
//TODO: this file needs to know the MapInstance's WorldSimulation rate - Maybe extract it as a configuration object ?

#define WORLD_UPDATE_TICKS_PER_SECOND 30

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

void abortLogout(Entity *e)
{
    e->m_logout_sent = false;
    e->m_is_logging_out = true; // send logout time of 0
    e->m_time_till_logout = 0;
}
