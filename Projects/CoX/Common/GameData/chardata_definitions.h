/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/NetStructures/SuperGroup.h"
#include "Common/NetStructures/Friend.h"
#include "Common/NetStructures/Powers.h"
#include "Common/GameData/attrib_definitions.h"
#include <QtCore/QString>
#include <vector>

enum SidekickType : uint8_t
{
    NoSidekick  = 0,
    IsSidekick  = 1,
    IsMentor    = 2,
};

struct Sidekick
{
static const constexpr  uint32_t        class_version   = 1;
                        bool            m_has_sidekick  = false;
                        uint32_t        m_db_id         = 0;
                        SidekickType    m_type          = NoSidekick;
};

struct CharacterData
{
static const constexpr  uint32_t    class_version       = 7; // v7: supergroupstats
                        uint32_t    m_level             = 0;
                        uint32_t    m_combat_level      = 0; // might be different if player is sidekick or exemplar, or hasn't trained up.
                        uint32_t    m_experience_points = 0;
                        uint32_t    m_experience_debt   = 0;
                        uint32_t    m_experience_patrol = 0; // planned future use
                        uint32_t    m_influence         = 0;
                        bool        m_has_titles        = false;
                        bool        m_has_the_prefix    = false;
                        QString     m_titles[3];             // Generic, Origin, Special
                        QString     m_battle_cry;
                        QString     m_character_description;
                        bool        m_afk               = false;
                        QString     m_afk_msg;
                        bool        m_lfg               = false;
                        QString     m_alignment         = "hero";
                        uint64_t    m_last_costume_id;
                        QString     m_last_online;
                        QString     m_class_name;
                        QString     m_origin_name;
                        Sidekick    m_sidekick;                 // Sidekick Struct
                        FriendsList m_friendlist;               // Friendslist Struct
                        SuperGroupStats     m_supergroup;       // This player's supergroupstats
                        Parse_CharAttrib    m_current_attribs;
                        vPowerSets          m_powersets;
                        PowerTrayGroup      m_trays;
                        vInspirations       m_inspirations;
                        vEnhancements       m_enhancements;     // Enhancements owned, but not attached to powers
                        uint32_t    m_max_insp_cols     = 0;
                        uint32_t    m_max_insp_rows     = 0;
                        uint32_t    m_max_enhance_slots = 0;

                        // No need to serialize these members
                        bool        m_powers_updated    = false;
                        bool        m_reset_powersets   = false;
};
