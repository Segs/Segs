/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#define _USE_MATH_DEFINES
#include "Entity.h"
#include "LFG.h"
#include "Team.h"
#include "Character.h"
#include "Servers/MapServer/DataHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/npc_definitions.h"
#include <QtCore/QDebug>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

//TODO: this file needs to know the MapInstance's WorldSimulation rate - Maybe extract it as a configuration object ?

#define WORLD_UPDATE_TICKS_PER_SECOND 30

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

void Entity::fillFromCharacter()
{
    m_hasname = !m_char->getName().isEmpty();
    m_entity_data.m_origin_idx = getEntityOriginIndex(true, getOrigin(*m_char));
    m_entity_data.m_class_idx = getEntityClassIndex(true, getClass(*m_char));
    m_is_hero = true;
}

/**
 *  This will mark the Entity as being in logging out state
 *  \arg time_till_logout is time in seconds untill logout is done
 */
void Entity::beginLogout(uint16_t time_till_logout)
{
    m_is_logging_out = true;
    m_time_till_logout = time_till_logout*1000;
    removeLFG(*this);
    leaveTeam(*this);
}

void fillEntityFromNewCharData(Entity &e, BitStream &src,const ColorAndPartPacker *packer,const Parse_AllKeyProfiles &default_profiles )
{
    QString description;
    QString battlecry;
    e.m_type = EntType(src.GetPackedBits(1));
    e.m_char->GetCharBuildInfo(src);
    e.m_char->recv_initial_costume(src,packer);
    e.m_char->m_char_data.m_has_the_prefix = src.GetBits(1); // The -> 1
    if(e.m_char->m_char_data.m_has_the_prefix)
        e.m_char->m_char_data.m_has_titles = true;
    src.GetString(battlecry);
    src.GetString(description);
    setBattleCry(*e.m_char,battlecry);
    setDescription(*e.m_char,description);
    e.m_entity_data.m_origin_idx = getEntityOriginIndex(true, getOrigin(*e.m_char));
    e.m_entity_data.m_class_idx = getEntityClassIndex(true, getClass(*e.m_char));
    e.m_player->m_keybinds.resetKeybinds(default_profiles);
    e.m_is_hero = true;

    // New Character Spawn Location
    e.m_entity_data.m_pos   = glm::vec3(-60.5f,0.0f,180.0f);    // Tutorial Starting Location
    //e.m_entity_data.m_pos = glm::vec3(128.0f,16.0f,-198.0f);  // Atlas Park Starting Location
    e.m_direction           = glm::quat(1.0f,0.0f,0.0f,0.0f);
}

const QString &Entity::name() const {
    return m_char->getName();
}

void Entity::dump()
{
    QString msg = "EntityDebug\n  "
            + name()
            + "\n  db_id: " + QString::number(m_db_id)
            + "\n  entity idx: " + QString::number(m_idx)
            + "\n  access level: " + QString::number(m_entity_data.m_access_level)
            + "\n  m_type: " + QString::number(uint8_t(m_type))
            + "\n  class idx: " + QString::number(m_entity_data.m_class_idx)
            + "\n  origin idx: " + QString::number(m_entity_data.m_origin_idx)
            + "\n  pos: " + QString::number(m_entity_data.m_pos.x) + ", "
                          + QString::number(m_entity_data.m_pos.y) + ", "
                          + QString::number(m_entity_data.m_pos.z)
            + "\n  orient: " + QString::number(m_entity_data.m_orientation_pyr.p) + ", "
                             + QString::number(m_entity_data.m_orientation_pyr.y) + ", "
                             + QString::number(m_entity_data.m_orientation_pyr.r)
            + "\n  target: " + QString::number(m_target_idx)
            + "\n  assist target: " + QString::number(m_assist_target_idx)
            + "\n  m_SG_id: " + QString::number(m_supergroup.m_SG_id);

    qDebug().noquote() << msg;

    if(m_team != nullptr)
        m_team->dump();

    if(m_type == EntType::PLAYER || m_type == EntType::NPC)
        m_char->dump();
    if(m_type == EntType::PLAYER)
        m_player->dump();
    dumpFriends(*this);
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
}

Entity::~Entity()
{

}

void abortLogout(Entity *e)
{
    e->m_is_logging_out = false; // send logout time of 0
    e->m_time_till_logout = 0;
}

void initializeNewPlayerEntity(Entity &e)
{
    e.m_costume_type                    = AppearanceType::WholeCostume;
    e.m_destroyed                       = false;
    e.m_type                            = EntType::PLAYER; // 2
    e.m_create_player                   = true;
    e.m_is_hero                         = true;
    e.m_is_villian                      = false;
    e.m_entity_data.m_origin_idx        = {0};
    e.m_entity_data.m_class_idx         = {0};
    e.m_hasname                         = true;
    e.m_has_supergroup                  = false;
    e.m_has_team                        = false;
    e.m_pchar_things                    = true;

    e.m_char.reset(new Character);
    e.m_player.reset(new PlayerData);
    e.m_player->reset();
    e.might_have_rare = e.m_rare_bits   = true;
}

void initializeNewNpcEntity(Entity &e,const Parse_NPC *src,int idx,int variant)
{
    e.m_costume_type                    = AppearanceType::NpcCostume;
    e.m_destroyed                       = false;
    e.m_type                            = EntType::NPC; // 2
    e.m_create_player                   = false;
    e.m_is_hero                         = false;
    e.m_is_villian                      = true;
    e.m_entity_data.m_origin_idx        = {0};
    e.m_entity_data.m_class_idx         = getEntityClassIndex(false,src->m_Class);
    e.m_hasname                         = true;
    e.m_has_supergroup                  = false;
    e.m_has_team                        = false;
    e.m_pchar_things                    = false;

    e.m_char.reset(new Character);
    e.m_npc.reset(new NPCData{false,src,idx,variant});
    e.m_player.reset();
    e.might_have_rare = e.m_rare_bits   = true;
}

void markEntityForDbStore(Entity *e, DbStoreFlags f)
{
    e->m_db_store_flags |= uint32_t(f);
}

//! @}
