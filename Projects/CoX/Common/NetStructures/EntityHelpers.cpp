/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
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
glm::vec3   getSpeed(const Entity &e) { return e.m_spd; }
float       getBackupSpd(const Entity &e) { return e.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }
Destination     getCurrentDestination(const Entity &e) { return e.m_cur_destination; }
ClientStates    getStateMode(const Entity &e) { return e.m_state_mode; }

// Setters
void setDbId(Entity &e, uint8_t val) { e.m_char->m_db_id = val; e.m_db_id = val; }
void setMapIdx(Entity &e, uint32_t val) { e.m_entity_data.m_map_idx = val; }
void setSpeed(Entity &e, float v1, float v2, float v3) { e.m_spd = {v1,v2,v3}; }
void setBackupSpd(Entity &e, float val) { e.m_backup_spd = val; }
void setJumpHeight(Entity &e, float val) { e.m_jump_height = val; }
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
void toggleFlying(Entity &e) { e.m_is_flying = !e.m_is_flying; }
void toggleFalling(Entity &e) { e.m_is_falling = !e.m_is_falling; }
void toggleJumping(Entity &e) { e.m_is_jumping = !e.m_is_jumping; }
void toggleSliding(Entity &e) { e.m_is_sliding = !e.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_is_stunned = !e.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_has_jumppack = !e.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }
void toggleInterp(Entity &e) { e.m_has_interp = !e.m_has_interp; }
void toggleMoveInstantly(Entity &e) { e.m_move_instantly = !e.m_move_instantly; }
void toggleTeamBuffs(PlayerData &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }

void toggleCollision(Entity &e)
{
    e.inp_state.m_no_collision = !e.inp_state.m_no_collision;

    if (e.inp_state.m_no_collision)
        e.m_move_type |= MoveType::MOVETYPE_NOCOLL;
    else
        e.m_move_type &= ~MoveType::MOVETYPE_NOCOLL;
     qDebug() << "Collision =" << QString::number(e.m_move_type, 2) << e.inp_state.m_no_collision;
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
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.25);
        break;
    case ReviveLevel::BOUNCE_BACK:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.5);
        break;
    case ReviveLevel::RESTORATION:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.75);
        break;
    case ReviveLevel::IMMORTAL_RECOVERY:
        setMaxHP(*e.m_char);
        break;
    case ReviveLevel::REGEN_REVIVE:
        setHP(*e.m_char, getMaxHP(*e.m_char)*0.75);
        setEnd(*e.m_char, getMaxEnd(*e.m_char)*0.5);
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
