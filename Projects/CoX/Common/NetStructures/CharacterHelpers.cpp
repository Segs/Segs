#include "CharacterHelpers.h"

#include "Character.h"

/*
 * Character Methods
 */
// Getter
uint32_t            getLevel(const Character &c) { return c.m_char_data.m_level; }
uint32_t            getCombatLevel(const Character &c) { return c.m_char_data.m_combat_level; }
float               getHP(const Character &c) { return c.m_char_data.m_current_attribs.m_HitPoints; }
float               getEnd(const Character &c) { return c.m_char_data.m_current_attribs.m_Endurance; }
uint64_t            getLastCostumeId(const Character &c) { return c.m_char_data.m_last_costume_id; }
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

// Setters
void setLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_level = val - 1; // client stores lvl arrays starting at 0
    // TODO: run finalizelevel here, but requires MapClientSession
}

void setCombatLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_combat_level = val;
}

void setHP(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_HitPoints = std::max(0.0f, std::min(val,c.m_max_attribs.m_HitPoints));
}

void setEnd(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_Endurance = std::max(0.0f, std::min(val,c.m_max_attribs.m_Endurance));
}

void    setLastCostumeId(Character &c, uint64_t val) { c.m_char_data.m_last_costume_id = val; }

void setXP(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_points = val;
    for (auto const &lvl : c.m_other_attribs.m_ExperienceRequired)
    {
        if (val >= lvl && val < lvl + 1)
        {
            setLevel(c, lvl);
            // TODO: set max attribs based upon level.
        }
    }
}

void setDebt(Character &c, uint32_t val) { c.m_char_data.m_experience_debt = val; }

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

void setInf(Character &c, uint32_t val) { c.m_char_data.m_influence = val; }
void setDescription(Character &c, QString val) { c.m_char_data.m_character_description = val; }
void setBattleCry(Character &c, QString val) { c.m_char_data.m_battle_cry = val; }

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
