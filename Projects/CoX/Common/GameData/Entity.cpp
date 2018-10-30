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

void fillEntityFromNewCharData(Entity &e, BitStream &src,const GameDataStore &data)
{
    QString description;
    QString battlecry;
    e.m_type = EntType(src.GetPackedBits(1));
    e.m_char->GetCharBuildInfo(src);
    e.m_char->recv_initial_costume(src,data.getPacker());
    e.m_char->m_char_data.m_has_the_prefix = src.GetBits(1); // The -> 1
    if(e.m_char->m_char_data.m_has_the_prefix)
        e.m_char->m_char_data.m_has_titles = true;
    src.GetString(battlecry);
    src.GetString(description);
    setBattleCry(*e.m_char,battlecry);
    setDescription(*e.m_char,description);
    e.m_entity_data.m_origin_idx = getEntityOriginIndex(data,true, getOrigin(*e.m_char));
    e.m_entity_data.m_class_idx = getEntityClassIndex(data,true, getClass(*e.m_char));
    e.m_player->m_keybinds.resetKeybinds(data.m_keybind_profiles);
    e.m_is_hero = true;

    e.m_direction = glm::quat(1.0f,0.0f,0.0f,0.0f);
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
    e.m_target_idx                      = -1;
    e.m_assist_target_idx               = -1;
    e.m_move_type                       = MoveType::MOVETYPE_WALK;
    e.m_motion_state.m_is_falling       = true;

    e.m_char = std::make_unique<Character>();
    e.m_player = std::make_unique<PlayerData>();
    e.m_player->reset();
    e.m_entity = std::make_unique<EntityData>();
    e.m_update_anims = e.m_rare_update   = true;

    std::copy(g_world_surf_params, g_world_surf_params+2, e.m_motion_state.m_surf_mods);

    e.m_states.init(); // Initialize movement input state pointers
    e.m_states.current()->m_pos_start = e.m_states.current()->m_pos_end = e.m_entity_data.m_pos;

    PosUpdate p;
    for(int i = 0; i<64; i++)
    {
        // Get timestamp in ms
        auto now_ms = std::chrono::steady_clock::now().time_since_epoch().count();

        p.m_position = e.m_entity_data.m_pos;
        p.m_pyr_angles = e.m_entity_data.m_orientation_pyr;
        p.m_timestamp = now_ms;
        addPosUpdate(e, p);
    }
}

void initializeNewNpcEntity(const GameDataStore &data, Entity &e, const Parse_NPC *src, int idx, int variant)
{
    e.m_costume_type                    = AppearanceType::NpcCostume;
    e.m_destroyed                       = false;
    e.m_type                            = EntType::NPC; // 2
    e.m_create_player                   = false;
    e.m_is_hero                         = false;
    e.m_is_villian                      = true;
    e.m_entity_data.m_origin_idx        = {0};
    e.m_entity_data.m_class_idx         = getEntityClassIndex(data,false,src->m_Class);
    e.m_hasname                         = true;
    e.m_has_supergroup                  = false;
    e.m_has_team                        = false;
    e.m_pchar_things                    = false;
    e.m_faction_data.m_has_faction      = true;
    e.m_faction_data.m_rank             = src->m_Rank;
    e.m_target_idx                      = -1;
    e.m_assist_target_idx               = -1;
    e.m_move_type                       = MoveType::MOVETYPE_WALK;
    e.m_motion_state.m_is_falling       = true;

    e.m_char = std::make_unique<Character>();
    e.m_npc = std::make_unique<NPCData>(NPCData{false,src,idx,variant});
    e.m_player.reset();
    e.m_entity = std::make_unique<EntityData>();
    e.m_update_anims = e.m_rare_update   = true;
    e.m_char->m_char_data.m_level       = src->m_Level;

    std::copy(g_world_surf_params, g_world_surf_params+2, e.m_motion_state.m_surf_mods);

    e.m_states.init(); // Initialize movement input state pointers
    e.m_states.current()->m_pos_start = e.m_states.current()->m_pos_end = e.m_entity_data.m_pos;

    PosUpdate p;
    for(int i = 0; i<64; i++)
    {
        // Get timestamp in ms
        auto now_ms = std::chrono::steady_clock::now().time_since_epoch().count();

        p.m_position = e.m_entity_data.m_pos;
        p.m_pyr_angles = e.m_entity_data.m_orientation_pyr;
        p.m_timestamp = now_ms;
        addPosUpdate(e, p);
    }
}

void markEntityForDbStore(Entity *e, DbStoreFlags f)
{
    e->m_db_store_flags |= uint32_t(f);
}

void unmarkEntityForDbStore(Entity *e, DbStoreFlags f)
{
    e->m_db_store_flags &= ~uint32_t(f);
}
void setStateMode(Entity &e, ClientStates state)
{
    e.m_rare_update = true; // this must also be true for statemode to send
    e.m_has_state_mode = true;
    e.m_state_mode = state;
}

void revivePlayer(Entity &e, ReviveLevel lvl)
{
    float cur_hp = getHP(*e.m_char);
    if(e.m_type != EntType::PLAYER && cur_hp != 0)
        return;

    switch(lvl)
    {
    case ReviveLevel::AWAKEN:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.25f);
        break;
    case ReviveLevel::BOUNCE_BACK:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.5f);
        break;
    case ReviveLevel::RESTORATION:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.75f);
        break;
    case ReviveLevel::IMMORTAL_RECOVERY:
        setMaxHP(*e.m_char);
        break;
    case ReviveLevel::REGEN_REVIVE:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.75f);
        setEnd(*e.m_char, getMaxEnd(*e.m_char)*0.5f);
        break;
    case ReviveLevel::FULL:
    default:
        // Set HP and End to Max
        setMaxHP(*e.m_char);
        setMaxEnd(*e.m_char);
        break;
    }

    // reset state to simple
    setStateMode(e, ClientStates::SIMPLE);
}

//! @}
