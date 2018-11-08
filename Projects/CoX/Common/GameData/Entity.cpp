/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Entity.h"
#include "LFG.h"
#include "Team.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Servers/MapServer/DataHelpers.h"
#include "GameData/GameDataStore.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/npc_definitions.h"
#include <QtCore/QDebug>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <memory>

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
bool Entity::validTarget(StoredEntEnum target, bool Iamvillian)
{
    // first check if the target needs to be dead, and isn't dead yet
    if (target == StoredEntEnum::DeadPlayer || target == StoredEntEnum::DeadTeammate || target == StoredEntEnum::DeadVillain)
        if (m_char->getHealth() != 0.0)             // target.dead() maybe?
            return false;
    switch(target)
    {
        case StoredEntEnum::Any:
        case StoredEntEnum::Location:               // locations are always valid for this check
        case StoredEntEnum::Teleport:               // ditto
        case StoredEntEnum::Caster:                 // self is always a valid target
            return true;
        case StoredEntEnum::None:
            return false;
        case StoredEntEnum::DeadVillain:
        case StoredEntEnum::Enemy:
        case StoredEntEnum::Foe:
        case StoredEntEnum::NPC:                    // not sure if this is supposed to be neutral, couldn't find any usage
            return (Iamvillian != m_is_villian);    // if both are heroes, or both villians, not a valid foe
        case StoredEntEnum::DeadTeammate:
        case StoredEntEnum::Teammate:
        case StoredEntEnum::DeadOrAliveTeammate:    // will need to do a check for teams seperately
        case StoredEntEnum::Friend:
        case StoredEntEnum::Player:
        case StoredEntEnum::DeadPlayer:
            return (Iamvillian != m_is_hero);       // both have to be heroes, or both villians
    }
}

// checks a vector of possible targets
bool Entity::validTarget(std::vector<StoredEntEnum> targets, bool Iamvillian)
{
    if (targets.size() == 0)
        return false;

    for (uint i =0; i < targets.size(); ++i)
        if (validTarget(targets.at(i), Iamvillian))
            return true;                            // just needs 1 to be valid

    return false;
}

void Entity::fillFromCharacter(const GameDataStore &data)
{
    m_hasname = !m_char->getName().isEmpty();
    m_entity_data.m_origin_idx = getEntityOriginIndex(data,true, getOrigin(*m_char));
    m_entity_data.m_class_idx = getEntityClassIndex(data,true, getClass(*m_char));
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
}

const QString &Entity::name() const
{
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
            + "\n  mapidx: " + QString::number(m_entity_data.m_map_idx)
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

Entity::Entity()
{
}

Entity::~Entity()
{

}

//! @}
