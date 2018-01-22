/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "PlayerMethods.h"
#include "../Projects/CoX/Servers/AdminServer/AdminServer.h"
#include "../Projects/CoX/Servers/AdminServer/CharacterDatabase.h"

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e) { return e.m_idx; }
uint32_t    getDbId(const Entity &e) { return e.m_db_id; }
uint32_t    getTargetIdx(Entity &e) { return e.inp_state.m_target_idx; }
uint32_t    getAssistTargetIdx(Entity &e) { return e.inp_state.m_assist_target_idx; }

// Setters
void    setDbId(Entity &e, uint8_t val) { e.m_char.m_db_id = val; e.m_db_id = val; }
void    setBackupSpd(Entity &e, const float &val) { e.m_backup_spd = val; }
void    setJumpHeight(Entity &e, const float &val) { e.m_jump_height = val; }
void    setUpdateID(Entity &e, const uint8_t &val) { e.m_update_id = val;}
void    setu1(Entity &e, const int &val) { e.u1 = val; }
void    setu2(Entity &e, const int &val) { e.u2 = val; }
void    setu3(Entity &e, const int &val) { e.u3 = val; }
void    setu4(Entity &e, const int &val) { e.u4 = val; }
void    setu5(Entity &e, const int &val) { e.u5 = val; }
void    setu6(Entity &e, const int &val) { e.u6 = val; }

// Toggles
void    toggleFly(Entity &e) { e.m_is_flying = !e.m_is_flying; }

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

void    toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }

// Misc Methods
void charUpdateDB(Entity *e)
{
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
        char_db->update(e);
}


/*
 * Character Methods
 */
// Getter
uint32_t    getLevel(Character &c) { return c.m_char_data.m_level; }
uint32_t    getCombatLevel(Character &c) { return c.m_char_data.m_combat_level; }
uint32_t    getHP(Character &c) { return c.m_current_attribs.m_HitPoints; }
uint32_t    getEnd(Character &c) { return c.m_current_attribs.m_Endurance; }
uint64_t    getLastCostumeId(Character &c) { return c.m_char_data.m_last_costume_id; }
QString &   getOrigin(Character &c) { return c.m_char_data.m_origin_name; }
QString &   getClass(Character &c) { return c.m_char_data.m_class_name; }
QString &   getMapName(Character &c) { return c.m_char_data.m_mapName; }
uint32_t    getXP(Character &c) { return c.m_char_data.m_experience_points; }
uint32_t    getDebt(Character &c) { return c.m_char_data.m_experience_debt; }
uint32_t    getPatrolXP(Character &c) { return c.m_char_data.m_experience_patrol; }
QString &   getTitles(Character &c) { return c.m_char_data.m_titles[3]; }
uint32_t    getInf(Character &c) { return c.m_char_data.m_influence; }
QString &   getDescription(Character &c) { return c.m_char_data.m_character_description ; }
QString &   getBattleCry(Character &c) { return c.m_char_data.m_battle_cry; }
QString &   getAlignment(Character &c) { return c.m_char_data.m_alignment; }

// Setters
void setLevel(Character &c, uint32_t val)
{
    c.m_char_data.m_level = val;
    // TODO: set max attribs based upon level
    //MapServerData map_server_data;
    //c.m_experience_points = map_server_data.expForLevel(val);
}

void    setCombatLevel(Character &c, uint32_t val) { c.m_char_data.m_combat_level = val; }
void    setHP(Character &c, uint32_t val) { c.m_current_attribs.m_HitPoints = val; }
void    setEnd(Character &c, uint32_t val) { c.m_current_attribs.m_Endurance = val; }
void    setLastCostumeId(Character &c, uint64_t val) { c.m_char_data.m_last_costume_id = val; }
void    setMapName(Character &c, const QString &val) { c.m_char_data.m_mapName = val; }

void setXP(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_points = val;
    for(auto const& lvl: c.m_other_attribs.m_ExperienceRequired)
    {
        if(val>=lvl && val<lvl+1)
        {
            setLevel(c, lvl);
            // TODO: set max attribs based upon level.
        }
    }
}

void    setDebt(Character &c, uint32_t val) { c.m_char_data.m_experience_debt = val; }

void setTitles(Character &c, bool prefix, QString generic, QString origin, QString special)
{
    c.m_char_data.m_has_titles = prefix || !generic.isEmpty() || !origin.isEmpty() || special.isEmpty();
    if(!c.m_char_data.m_has_titles)
      return;

    c.m_char_data.m_has_the_prefix = prefix;
    c.m_char_data.m_titles[0] = generic;
    c.m_char_data.m_titles[1] = origin;
    c.m_char_data.m_titles[2] = special;
}

void    setInf(Character &c, uint32_t val) { c.m_char_data.m_influence = val; }
void    setDescription(Character &c, QString val) { c.m_char_data.m_character_description = val; }
void    setBattleCry(Character &c, QString val) { c.m_char_data.m_battle_cry = val; }

// Toggles
void toggleAFK(Character &c, const QString &msg)
{
    c.m_char_data.m_afk = !c.m_char_data.m_afk;
    if(c.m_char_data.m_afk)
        c.m_char_data.m_afk_msg = msg;
}

void    toggleLFG(Character &c) { c.m_char_data.m_lfg = !c.m_char_data.m_lfg; }
