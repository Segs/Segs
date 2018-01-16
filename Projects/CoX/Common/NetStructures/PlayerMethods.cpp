/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "PlayerMethods.h"

/*
 * Entity Methods
 */
uint32_t    getIdx(Entity &e) { return e.m_idx; }
uint32_t    getDbId(Entity &e) { return e.m_db_id; }
void        setDbId(Entity &e, uint8_t val) { e.m_char.m_db_id = val; e.m_db_id = val; }

void    toggleFly(Entity &e) { e.m_is_flying = !e.m_is_flying; }
void    setBackupSpd(Entity &e, const float &val) { e.m_backup_spd = val; }
void    setJumpHeight(Entity &e, const float &val) { e.m_jump_height = val; }
void    toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void    setUpdateID(Entity &e, const uint8_t &val) { e.m_update_id = val;}
void    setu1(Entity &e, const int &val) { e.u1 = val; }
void    setu2(Entity &e, const int &val) { e.u2 = val; }
void    setu3(Entity &e, const int &val) { e.u3 = val; }
void    setu4(Entity &e, const int &val) { e.u4 = val; }
void    setu5(Entity &e, const int &val) { e.u5 = val; }
void    setu6(Entity &e, const int &val) { e.u6 = val; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }


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

void charUpdateDB(Entity *e)
{
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
        char_db->update(&e->m_char);
}

/*
 * Character Methods
 */
uint32_t            getLevel(Character &src) { return src.m_level; }
uint32_t            getHP(Character &src) { return src.m_current_attribs.m_HitPoints; }
void                setHP(Character &src, uint32_t val) { src.m_current_attribs.m_HitPoints = val; }
uint32_t            getEnd(Character &src) { return src.m_current_attribs.m_Endurance; }
void                setEnd(Character &src, uint32_t val) { src.m_current_attribs.m_Endurance = val; }
uint64_t            getLastCostumeId(Character &src) { return src.m_last_costume_id; }
void                setLastCostumeId(Character &src, uint64_t val) { src.m_last_costume_id = val; }
const QString &     getOrigin(Character &src) { return src.m_origin_name; }
const QString &     getClass(Character &src) { return src.m_class_name; }
const QString &     getMapName(Character &src) { return src.m_mapName; }
void                setMapName(Character &src, const QString &val) { src.m_mapName = val; }
uint32_t            getXP(Character &src) { return src.m_experience_points; }
uint32_t            getDebt(Character &src) { return src.m_experience_debt; }
void                setDebt(Character &src, uint32_t val) { src.m_experience_debt = val; }
const QString &     getTitles(Character &src) { return src.m_titles[3]; }
void                toggleLFG(Character &src) { src.m_lfg = !src.m_lfg; }
uint32_t            getInf(Character &src) { return src.m_influence; }
void                setInf(Character &src, uint32_t val) { src.m_influence = val; }

void toggleAFK(Character &src, const QString &msg)
{
    src.m_afk = !src.m_afk;
    if(src.m_afk)
        src.m_afk_msg = msg;
}

void setXP(Character &src, uint32_t val)
{
    src.m_experience_points = val;
    for(auto const& lvl: src.m_other_attribs.m_ExperienceRequired)
    {
        if(val>=lvl && val<lvl+1)
        {
            setLevel(src, lvl);
            // TODO: set max attribs based upon level.
        }
    }
}

void setLevel(Character &src, uint32_t val)
{
    src.m_level = val;
    // TODO: set max attribs based upon level
    //MapServerData map_server_data;
    //src.m_experience_points = map_server_data.expForLevel(val);
}

void setTitles(Character &src, bool prefix, QString generic, QString origin, QString special)
{
    src.m_has_titles = prefix || !generic.isEmpty() || !origin.isEmpty() || special.isEmpty();
    if(!src.m_has_titles)
      return;

    src.m_has_the_prefix = prefix;
    src.m_titles[0] = generic;
    src.m_titles[1] = origin;
    src.m_titles[2] = special;
}
