#pragma once

#include <QtCore/QString>

struct CharacterData
{
    uint32_t    m_level             = 0;
    uint32_t    m_combat_level      = 0; // might be different if player is sidekick or exemplar, or hasn't trained up.
    uint32_t    m_experience_points = 0;
    uint32_t    m_experience_debt   = 0;
    uint32_t    m_experience_patrol = 0; // planned future use
    uint32_t    m_influence         = 0;
    bool        m_has_titles;
    bool        m_has_the_prefix;
    QString     m_titles[3];             // Generic, Origin, Special
    QString     m_battle_cry;
    QString     m_character_description;
    bool        m_afk               = false;
    QString     m_afk_msg;
    uint8_t     m_lfg               = 0;
    QString     m_alignment         = "hero";
    uint64_t    m_last_costume_id;
    QString     m_last_online;
    QString     m_class_name;
    QString     m_origin_name;
    QString     m_mapName;
    bool        m_supergroup_costume; // player has a sg costume
    bool        m_using_sg_costume;   // player uses sg costume currently
    uint8_t     m_cur_chat_channel          = 10; // Default is local
};
