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

#include "CharacterHelpers.h"
#include "Character.h"
#include "Costume.h"
#include <QDateTime>

/*
 * Character Methods
 */
// Getter
uint32_t            getLevel(const Character &c) { return c.m_char_data.m_level; }
uint32_t            getCombatLevel(const Character &c) { return c.m_char_data.m_combat_level; }
uint32_t            getSecurityThreat(const Character &c) { return c.m_char_data.m_security_threat; }
float               getHP(const Character &c) { return c.m_char_data.m_current_attribs.m_HitPoints; }
float               getEnd(const Character &c) { return c.m_char_data.m_current_attribs.m_Endurance; }
float               getMaxHP(const Character &c) { return c.m_max_attribs.m_HitPoints; }
float               getMaxEnd(const Character &c) { return c.m_max_attribs.m_Endurance; }
uint32_t            getCurrentCostumeIdx(const Character &c) { return c.m_char_data.m_current_costume_idx; }
const QString &     getOrigin(const Character &c) { return c.m_char_data.m_origin_name; }
const QString &     getClass(const Character &c) { return c.m_char_data.m_class_name; }
uint32_t            getXP(const Character &c) { return c.m_char_data.m_experience_points; }
uint32_t            getDebt(const Character &c) { return c.m_char_data.m_experience_debt; }
uint32_t            getPatrolXP(const Character &c) { return c.m_char_data.m_experience_patrol; }
const QString &     getGenericTitle(const Character &c) { return c.m_char_data.m_titles[0]; }
const QString &     getOriginTitle(const Character &c) { return c.m_char_data.m_titles[1]; }
const QString &     getSpecialTitle(const Character &c) { return c.m_char_data.m_titles[2]; }
uint32_t            getInf(const Character &c) { return c.m_char_data.m_influence; }
const QString &     getDescription(const Character &c) { return c.m_char_data.m_character_description ; }
const QString &     getBattleCry(const Character &c) { return c.m_char_data.m_battle_cry; }
const QString &     getAlignment(const Character &c) { return c.m_char_data.m_alignment; }
const QString &     getLastOnline(const Character &c) { return c.m_char_data.m_last_online; }

// Setters
void setLevel(Character &c, uint32_t val)
{
    GameDataStore &data(getGameData());
    if(val > data.expMaxLevel())
        val = data.expMaxLevel();
    c.m_char_data.m_level = val; // client stores lvl arrays starting at 0
    c.finalizeLevel();
    setHPToMax(c);
    setEndToMax(c);
}

void setCombatLevel(Character &c, uint32_t val)
{
    GameDataStore &data(getGameData());
    if(val > data.expMaxLevel())
        val = data.expMaxLevel();
    c.m_char_data.m_combat_level = val;
    c.finalizeCombatLevel();
}

void setSecurityThreat(Character &c, uint32_t val)
{
    GameDataStore &data(getGameData());
    if(val > data.expMaxLevel())
        val = data.expMaxLevel();
    c.m_char_data.m_security_threat = val;
}

void setHP(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_HitPoints = std::max(0.0f, std::min(val,c.m_max_attribs.m_HitPoints));
}

void setHPToMax(Character &c)
{
    setHP(c, getMaxHP(c));
}

void setEnd(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_Endurance = std::max(0.0f, std::min(val,c.m_max_attribs.m_Endurance));
}

void setEndToMax(Character &c)
{
    setEnd(c, getMaxEnd(c));
}

void setCurrentCostumeIdx(Character &c, uint32_t idx)
{
    c.m_add_new_costume = true;
    c.m_char_data.m_current_costume_idx = idx;
}

void setXP(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_points = val;
}

void setDebt(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_debt = val;
}

void setTitles(Character &c, bool prefix, QString generic, QString origin, QString special)
{
    // if "NULL", clear string
    if(generic=="NULL")
        generic.clear();
    if(origin=="NULL")
        origin.clear();
    if(special=="NULL")
        special.clear();

    c.m_char_data.m_has_titles = prefix || !generic.isEmpty() || !origin.isEmpty() || !special.isEmpty();
    if(!c.m_char_data.m_has_titles)
      return;

    c.m_char_data.m_has_the_prefix = prefix;
    c.m_char_data.m_titles[0] = generic;
    c.m_char_data.m_titles[1] = origin;
    c.m_char_data.m_titles[2] = special;
}

void setInf(Character &c, uint32_t val)
{
    c.m_char_data.m_influence = val;
}

void setDescription(Character &c, QString val)
{
    c.m_char_data.m_character_description = val;
}

void setBattleCry(Character &c, QString val)
{
    c.m_char_data.m_battle_cry = val;
}

void initializeCharacter(Character &c)
{
    GameDataStore &data(getGameData());

    uint entclass = getEntityClassIndex(data, true, c.m_char_data.m_class_name);
    c.m_char_data.m_current_attribs.m_ToHit             = data.m_player_classes[entclass].m_AttribBase[0].m_ToHit;
    c.m_char_data.m_current_attribs.m_Defense           = data.m_player_classes[entclass].m_AttribBase[0].m_Defense;
    c.m_char_data.m_current_attribs.m_Evade             = data.m_player_classes[entclass].m_AttribBase[0].m_Evade;
    c.m_char_data.m_current_attribs.m_SpeedRunning      = data.m_player_classes[entclass].m_AttribBase[0].m_SpeedRunning;
    c.m_char_data.m_current_attribs.m_SpeedFlying       = data.m_player_classes[entclass].m_AttribBase[0].m_SpeedFlying;
    c.m_char_data.m_current_attribs.m_SpeedSwimming     = data.m_player_classes[entclass].m_AttribBase[0].m_SpeedSwimming;
    c.m_char_data.m_current_attribs.m_SpeedJumping      = data.m_player_classes[entclass].m_AttribBase[0].m_SpeedJumping;
    c.m_char_data.m_current_attribs.m_jump_height       = data.m_player_classes[entclass].m_AttribBase[0].m_jump_height;
    c.m_char_data.m_current_attribs.m_MovementControl   = data.m_player_classes[entclass].m_AttribBase[0].m_MovementControl;
    c.m_char_data.m_current_attribs.m_MovementFriction  = data.m_player_classes[entclass].m_AttribBase[0].m_MovementFriction;
    c.m_char_data.m_current_attribs.m_Stealth           = data.m_player_classes[entclass].m_AttribBase[0].m_Stealth;
    c.m_char_data.m_current_attribs.m_StealthRadius     = data.m_player_classes[entclass].m_AttribBase[0].m_StealthRadius;
    c.m_char_data.m_current_attribs.m_PerceptionRadius  = data.m_player_classes[entclass].m_AttribBase[0].m_PerceptionRadius;
    c.m_char_data.m_current_attribs.m_Regeneration      = data.m_player_classes[entclass].m_AttribBase[0].m_Regeneration*4;//for some reason default is 0.25
    c.m_char_data.m_current_attribs.m_Recovery          = data.m_player_classes[entclass].m_AttribBase[0].m_Regeneration;
    c.m_char_data.m_current_attribs.m_ThreatLevel       = data.m_player_classes[entclass].m_AttribBase[0].m_ThreatLevel;
    c.m_char_data.m_current_attribs.m_Taunt             = data.m_player_classes[entclass].m_AttribBase[0].m_Taunt;
    c.m_char_data.m_current_attribs.m_Confused          = data.m_player_classes[entclass].m_AttribBase[0].m_Confused;
    c.m_char_data.m_current_attribs.m_Afraid            = data.m_player_classes[entclass].m_AttribBase[0].m_Afraid;
    c.m_char_data.m_current_attribs.m_Held              = data.m_player_classes[entclass].m_AttribBase[0].m_Held;
    c.m_char_data.m_current_attribs.m_Immobilized       = data.m_player_classes[entclass].m_AttribBase[0].m_Immobilized;
    c.m_char_data.m_current_attribs.m_is_stunned        = data.m_player_classes[entclass].m_AttribBase[0].m_is_stunned;
    c.m_char_data.m_current_attribs.m_Sleep             = data.m_player_classes[entclass].m_AttribBase[0].m_Sleep;
    c.m_char_data.m_current_attribs.m_is_flying         = data.m_player_classes[entclass].m_AttribBase[0].m_is_flying;
    c.m_char_data.m_current_attribs.m_has_jumppack      = data.m_player_classes[entclass].m_AttribBase[0].m_has_jumppack;
    c.m_char_data.m_current_attribs.m_Teleport          = data.m_player_classes[entclass].m_AttribBase[0].m_Teleport;
    c.m_char_data.m_current_attribs.m_Untouchable       = data.m_player_classes[entclass].m_AttribBase[0].m_Untouchable;
    c.m_char_data.m_current_attribs.m_Intangible        = data.m_player_classes[entclass].m_AttribBase[0].m_Intangible;
    c.m_char_data.m_current_attribs.m_OnlyAffectsSelf   = data.m_player_classes[entclass].m_AttribBase[0].m_OnlyAffectsSelf;
    c.m_char_data.m_current_attribs.m_Knockup           = data.m_player_classes[entclass].m_AttribBase[0].m_Knockup;
    c.m_char_data.m_current_attribs.m_Knockback         = data.m_player_classes[entclass].m_AttribBase[0].m_Knockback;
    c.m_char_data.m_current_attribs.m_Repel             = data.m_player_classes[entclass].m_AttribBase[0].m_Repel;
    c.m_char_data.m_current_attribs.m_Accuracy          = data.m_player_classes[entclass].m_AttribBase[0].m_Accuracy;
    c.m_char_data.m_current_attribs.m_Radius            = data.m_player_classes[entclass].m_AttribBase[0].m_Radius;
    c.m_char_data.m_current_attribs.m_Arc               = data.m_player_classes[entclass].m_AttribBase[0].m_Arc;
    c.m_char_data.m_current_attribs.m_Range             = data.m_player_classes[entclass].m_AttribBase[0].m_Range;
    c.m_char_data.m_current_attribs.m_TimeToActivate    = data.m_player_classes[entclass].m_AttribBase[0].m_TimeToActivate;
    c.m_char_data.m_current_attribs.m_RechargeTime      = data.m_player_classes[entclass].m_AttribBase[0].m_RechargeTime;
    c.m_char_data.m_current_attribs.m_InterruptTime     = data.m_player_classes[entclass].m_AttribBase[0].m_InterruptTime;
    c.m_char_data.m_current_attribs.m_EnduranceDiscount = data.m_player_classes[entclass].m_AttribBase[0].m_EnduranceDiscount;
}

void updateLastOnline(Character &c)
{
    c.m_char_data.m_last_online = QDateTime::currentDateTime().toString();
}

// Toggles
void toggleAFK(Character &c, const bool isTrue, QString msg)
{
    c.m_char_data.m_afk = isTrue;
    //c.m_char_data.m_afk = !c.m_char_data.m_afk;
    if(c.m_char_data.m_afk)
        c.m_char_data.m_afk_msg = msg; 
}

void toggleAFK(Character &c, QString msg)
{
    toggleAFK(c, !c.m_char_data.m_afk, msg);
}


/*
 * Titles -- TODO: get titles from texts/English/titles_def
 */
static const QStringList g_generic_titles =
{
    "NULL",
    "Awesome",
    "Bold",
    "Courageous",
    "Daring",
    "Extraordinary",
    "Famous",
    "Gallant",
    "Heroic",
    "Incomparable",
    "Legendary",
    "Magnificent",
    "Outstanding",
    "Powerful",
    "Remarkable",
    "Startling",
    "Terrific",
    "Ultimate",
    "Valiant",
    "Wonderful",
};

// TODO: get titles from texts/English/titles_def
static const QStringList g_origin_titles =
{
    "NULL",
    "Adept",
    "Bright",
    "Curious",
    "Deductiv",
    "Exceptional",
    "Far Seeing",
    "Glorious",
    "Honorable",
    "Indescribable",
    "Lucky",
    "Majestic",
    "Otherworldly",
    "Phenomenal",
    "Redoubtable",
    "Stupendous",
    "Thoughtful",
    "Unearthly",
    "Venturous",
    "Watchful",
};

const QString &getGenericTitle(uint32_t val)
{
    return g_generic_titles.at(val);
}

const QString &getOriginTitle(uint32_t val)
{
    return g_origin_titles.at(val);
}

//! @}
