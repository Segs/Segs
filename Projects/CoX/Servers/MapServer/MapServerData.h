/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/CoXHash.h"
#include "Common/GameData/costume_definitions.h"
#include "Common/GameData/origin_definitions.h"
#include "Common/GameData/charclass_definitions.h"
#include "Common/GameData/keybind_definitions.h"
#include "Common/GameData/other_definitions.h"
#include "Common/GameData/npc_definitions.h"
#include "Common/GameData/power_definitions.h"

#include "NpcStore.h"

class ColorAndPartPacker;
class QString;

class MapServerData
{
        ColorAndPartPacker *packer_instance;
        LevelExpAndDebt     m_experience_and_debt_per_level;

        bool            read_costumes(const QString &directory_path);
        bool            read_colors(const QString &src_filename);
        bool            read_origins(const QString &src_filename);
        bool            read_classes(const QString &src_filename);
        bool            read_exp_and_debt(const QString &src_filename);
        bool            read_keybinds(const QString &src_filename);
        bool            read_commands(const QString &src_filename);
        bool            read_npcs(const QString &directory_path);
        bool            read_settings(const QString &directory_path);
        bool            read_powers(const QString &directory_path);
        bool            read_combine_chances(const QString &directory_path);
        bool            read_effectiveness(const QString &directory_path);
        bool            read_pi_schedule(const QString &directory_path);
public:
                        MapServerData();
                        ~MapServerData();
        bool            read_runtime_data(const QString &directory_path);
        const ColorAndPartPacker *getPacker() const { return packer_instance; }
        int             expForLevel(int lev) const;
        int             expDebtForLevel(int lev) const;
        int             expMaxLevel();
        int             countForLevel(int lvl, std::vector<uint32_t> &schedule) const;
        const NPCStorage & getNPCDefinitions() const
                        {
                            return m_npc_store;
                        }
        Pallette_Data               m_supergroup_colors;
        CostumeSet_Data             m_costume_store;
        Parse_AllOrigins            m_player_origins;
        Parse_AllOrigins            m_other_origins;
        Parse_AllCharClasses        m_player_classes;
        Parse_AllCharClasses        m_other_classes;
        Parse_AllKeyProfiles        m_keybind_profiles;
        Parse_AllCommandCategories  m_command_categories;
        NPCStorage                  m_npc_store;
        AllPowerCategories          m_all_powers;
        Parse_Combining             m_combine_chances;
        Parse_Combining             m_combine_same;
        Parse_Effectiveness         m_effectiveness_above;
        Parse_Effectiveness         m_effectiveness_below;
        Parse_PI_Schedule           m_pi_schedule;
        float                       m_player_fade_in;

        // auto-AFK and logout settings, auto-AFK is mandatory, server can choose between auto-logout or not
        float                       m_time_to_afk = 30;          // 30 secs
        // when player has reached the threshold for m_time_to_logout, they will be notified that they will be
        // auto-logged out in m_time_to_auto_logout seconds
        // so m_time_to_logout_msg + m_time_to_auto_logout is when the player will be kicked out of the server
        float                       m_time_to_logout_msg = 45;  // 60 secs
        float                       m_time_to_auto_logout = 60;
        float                       m_uses_auto_logout = true;
};
