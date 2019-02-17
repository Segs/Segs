/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "EntityHelpers.h"
#include "Entity.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Powers.h"
#include "Logging.h"
#include "GameData/playerdata_definitions.h"

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e) { return e.m_idx; }
uint32_t    getDbId(const Entity &e) { return e.m_db_id; }
uint32_t    getAccessLevel(const Entity &e) { return e.m_entity_data.m_access_level; }
uint32_t    getTargetIdx(const Entity &e) { return e.m_target_idx; }
uint32_t    getAssistTargetIdx(const Entity &e) { return e.m_assist_target_idx; }
glm::vec3   getSpeed(const Entity &e) { return e.m_motion_state.m_speed; }
float       getBackupSpd(const Entity &e) { return e.m_motion_state.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_motion_state.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }
Destination     getCurrentDestination(const Entity &e) { return e.m_cur_destination; }
ClientStates    getStateMode(const Entity &e) { return e.m_state_mode; }

// Setters
void setDbId(Entity &e, uint8_t val) { e.m_char->m_db_id = val; e.m_db_id = val; }
void setMapIdx(Entity &e, uint32_t val) { e.m_entity_data.m_map_idx = val; }
void setSpeed(Entity &e, float v1, float v2, float v3) { e.m_motion_state.m_speed = {v1,v2,v3}; }
void setBackupSpd(Entity &e, float val) { e.m_motion_state.m_backup_spd = val; }
void setJumpHeight(Entity &e, float val) { e.m_motion_state.m_jump_height = val; }
void setUpdateID(Entity &e, uint8_t val) { e.m_update_id = val;}

void setTeamID(Entity &e, uint8_t team_id)
{
    if(team_id == 0)
    {
        e.m_has_team            = false;
        delete e.m_team;
        e.m_team = nullptr;
    }
    else
        e.m_has_team            = true;

    if(!e.m_team)
        return;

    qDebug().noquote() << "Team Info:"
                       << "\n  Has Team:" << e.m_has_team
                       << "\n  ID:" << e.m_team->m_team_idx
                       << "\n  Size:" << e.m_team->m_team_members.size()
                       << "\n  Members:" << e.m_team->m_team_members.data();
}

void setSuperGroup(Entity &e, int sg_id, QString sg_name, uint32_t sg_rank)
{
    // TODO: provide method for updating SuperGroup Colors
    if(sg_id == 0)
    {
        e.m_has_supergroup          = false;
        e.m_supergroup.m_SG_id      = 0;
        e.m_supergroup.m_SG_name    = "";
        e.m_supergroup.m_SG_color1  = 0x996633FF;
        e.m_supergroup.m_SG_color2  = 0x336699FF;
        e.m_supergroup.m_SG_rank    = 0;
    }
    else
    {
        e.m_has_supergroup          = true;
        e.m_supergroup.m_SG_id      = sg_id;
        e.m_supergroup.m_SG_name    = sg_name;
        e.m_supergroup.m_SG_color1  = 0xAA3366FF;
        e.m_supergroup.m_SG_color2  = 0x66AA33FF;
        e.m_supergroup.m_SG_rank    = sg_rank;
    }
    qDebug().noquote() << "SG Info:"
             << "\n  Has Team:" << e.m_has_supergroup
             << "\n  ID:" << e.m_supergroup.m_SG_id
             << "\n  Name:" << e.m_supergroup.m_SG_name
             << "\n  Color1:" << e.m_supergroup.m_SG_color1
             << "\n  Color2:" << e.m_supergroup.m_SG_color2
             << "\n  Rank:" << e.m_supergroup.m_SG_rank;
}

void setTarget(Entity &e, uint32_t target_idx)
{
    // TODO: set target if enemy, set assist_target if friendly
    e.m_target_idx = target_idx;
    // To trigger update to client
    e.m_pchar_things = true;
}

void setAssistTarget(Entity &e, uint32_t target_idx)
{
    // TODO: it appears as thogh assist_target is friendly targets only
    // not the target of your target, which is derived when using the
    // slash command /assist
    e.m_assist_target_idx = target_idx;
    qCDebug(logTarget) << "Assist Target is:" << getAssistTargetIdx(e);
}

void setCurrentDestination(Entity &e, int point_idx, glm::vec3 location)
{
    e.m_cur_destination.point_idx = point_idx;
    e.m_cur_destination.location = location;
}

void setStateMode(Entity &e, ClientStates state)
{
    e.m_rare_update = true; // this must also be true for statemode to send
    e.m_has_state_mode = true;
    e.m_state_mode = state;
}

// For live debugging
void    setu1(Entity &e, int val) { e.u1 = val; }

// Toggles
void toggleFlying(Entity &e) { e.m_motion_state.m_is_flying = !e.m_motion_state.m_is_flying; }
void toggleFalling(Entity &e) { e.m_motion_state.m_is_falling = !e.m_motion_state.m_is_falling; }
void toggleJumping(Entity &e) { e.m_motion_state.m_is_jumping = !e.m_motion_state.m_is_jumping; }
void toggleSliding(Entity &e) { e.m_motion_state.m_is_sliding = !e.m_motion_state.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_motion_state.m_is_stunned = !e.m_motion_state.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_motion_state.m_has_jumppack = !e.m_motion_state.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void toggleControlsDisabled(Entity &e) { e.m_motion_state.m_controls_disabled = !e.m_motion_state.m_controls_disabled; }
void toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }
void toggleInterp(Entity &e) { e.m_has_interp = !e.m_has_interp; }
void toggleMoveInstantly(Entity &e) { e.m_move_instantly = !e.m_move_instantly; }
void toggleTeamBuffs(PlayerData &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }

void toggleCollision(Entity &e)
{
    e.m_motion_state.m_no_collision = !e.m_motion_state.m_no_collision;

    if(e.m_motion_state.m_no_collision)
        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;

    qDebug() << "Collision =" << QString::number(e.m_move_type, 2) << e.m_motion_state.m_no_collision;
}

void toggleMovementAuthority(Entity &e)
{
    toggleFullUpdate(e);
    toggleControlId(e);
}


// Misc Methods
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
    e.m_is_villian                      = false; // only Critters are Villains
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

void initializeNewCritterEntity(const GameDataStore &data, Entity &e, const Parse_NPC *src, int idx, int variant, int level)
{
    e.m_costume_type                    = AppearanceType::NpcCostume;
    e.m_destroyed                       = false;
    e.m_type                            = EntType::CRITTER;
    e.m_create_player                   = false;
    e.m_is_hero                         = false;
    e.m_is_villian                      = true;
    e.m_entity_data.m_origin_idx        = {0};
    e.m_entity_data.m_class_idx         = getEntityClassIndex(data,false,src->m_Class);
    e.m_hasname                         = true;
    e.m_has_supergroup                  = false;
    e.m_has_team                        = false;
    e.m_pchar_things                    = true;
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

    e.m_char->m_char_data.m_combat_level = level;
    e.m_char->m_char_data.m_level = level;
    e.m_char->m_char_data.m_security_threat = level;

    // Should pull attributes for critter at X level from DB or Scripts?
    e.m_char->m_max_attribs.m_HitPoints = 100;
    e.m_char->m_max_attribs.m_Endurance = 100;
    e.m_char->m_char_data.m_current_attribs.m_HitPoints = 100;
    e.m_char->m_char_data.m_current_attribs.m_Endurance = 100;

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

void markEntityForDbStore(Entity *e, DbStoreFlags f)
{
    e->m_db_store_flags |= uint32_t(f);
}

void unmarkEntityForDbStore(Entity *e, DbStoreFlags f)
{
    e->m_db_store_flags &= ~uint32_t(f);
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
        setHPToMax(*e.m_char);
        break;
    case ReviveLevel::REGEN_REVIVE:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.75f);
        setEnd(*e.m_char, getMaxEnd(*e.m_char)*0.5f);
        break;
    case ReviveLevel::FULL:
    default:
        // Set HP and End to Max
        setHPToMax(*e.m_char);
        setEndToMax(*e.m_char);
        break;
    }

    // reset state to simple
    setStateMode(e, ClientStates::SIMPLE);
}

//! @}
