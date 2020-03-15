/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/Sidekick.h"
#include "Common/GameData/Friend.h"
#include "Common/GameData/Powers.h"

#include <QString>
#include <stdint.h>


struct CharacterData
{
        enum : uint32_t {class_version       = 12}; // v12: Moved Contacts/Clues/Souviners/Tasks to PlayerData

        QString     m_titles[3];             // Generic, Origin, Special
        QString     m_class_name;
        QString     m_origin_name;
        QString     m_battle_cry;
        QString     m_character_description;
        QString     m_afk_msg;
        QString     m_alignment         = "hero";
        QString     m_last_online;
        uint32_t    m_current_costume_idx = 0;
        uint32_t    m_level             = 0;
        uint32_t    m_combat_level      = 0; // might be different if player is sidekick or exemplar.
        uint32_t    m_security_threat   = 0; // used to limit a character's participation in Task Forces, Strike Forces, and Trials
        uint32_t    m_experience_points = 0;
        uint32_t    m_experience_debt   = 0;
        uint32_t    m_experience_patrol = 0; // planned future use
        uint32_t    m_influence         = 0;
        Sidekick    m_sidekick;                 // Sidekick Struct
        FriendsList m_friendlist;               // Friendslist Struct
        Parse_CharAttrib m_current_attribs;
        float       m_idle_time         = 0;
        vPowerSets          m_powersets;
        PowerTrayGroup      m_trays;
        vInspirations       m_inspirations;
        vEnhancements       m_enhancements;     // Enhancements owned, but not attached to powers

        uint32_t    m_max_insp_cols     = 0;
        uint32_t    m_max_insp_rows     = 0;
        uint32_t    m_max_enhance_slots = 0;

        bool        m_has_titles        = false;
        bool        m_has_the_prefix    = false;
        bool        m_afk               = false;
        bool        m_lfg               = false;
        bool        m_has_sg_costume;           // player has a supergroup costume
        bool        m_using_sg_costume;         // player uses sg costume currently
        bool        m_is_on_task_force  = false;
        bool        m_is_on_auto_logout = false;
        // No need to serialize these members
        bool        m_has_updated_powers    = false;
        bool        m_reset_powersets   = false;
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

