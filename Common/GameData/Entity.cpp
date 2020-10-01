/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Entity.h"
#include "EntityHelpers.h"
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

#include <memory>

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
    m_entity_update_flags.setFlag(UpdateFlag::LOGOUT);
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

void Entity::setActiveDialogCallback(std::function<void(int)> callback)
{
    this->m_active_dialog = callback;
}

Entity::Entity()
{
}

Entity::~Entity()
{

}

//! @}
