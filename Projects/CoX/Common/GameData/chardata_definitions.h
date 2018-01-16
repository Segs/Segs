#pragma once

#include <QString>

class Parse_CharAttrib;
class LevelExpAndDebt;

struct CharacterData
{
//    Parse_CharAttrib    m_current_attribs;
//    Parse_CharAttrib    m_max_attribs;
//    LevelExpAndDebt     m_other_attribs;
    uint32_t            m_level             = 0;
    uint32_t            m_combat_level      = 0; // might be different if player is sidekick or exemplar, or hasn't trained up.
    uint32_t            m_experience_points = 0;
    uint32_t            m_experience_debt   = 0;
    uint32_t            m_experience_patrol = 0;
    uint32_t            m_influence         = 1;
    bool                m_has_titles        = false;
    bool                m_has_the_prefix    = false;
    QString             m_titles[3];        // Generic, Origin, Special
    QString             m_battle_cry;
    QString             m_character_description;
    bool                m_afk               = false;
    QString             m_afk_msg;
    uint8_t             m_lfg               = 0;
    uint32_t            m_supergroup_id     = 0;
    QString             m_alignment         = "hero";
    uint64_t            m_last_costume_id;
    QString             m_class_name;
    QString             m_origin_name;
    QString             m_mapName;
    uint32_t            m_account_id;
    uint32_t            m_db_id;
};
