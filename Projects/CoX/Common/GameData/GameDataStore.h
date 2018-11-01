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
#include "Common/GameData/CharacterClass.h"
#include "Common/GameData/keybind_definitions.h"
#include "Common/GameData/other_definitions.h"
#include "Common/GameData/npc_definitions.h"
#include "Common/GameData/power_definitions.h"

#include "NpcStore.h"

class ColorAndPartPacker;
class IndexedStringPacker;
class QString;
class GameDataStore
{
        ColorAndPartPacker * packer_instance      = nullptr;
        IndexedStringPacker *m_index_based_packer = nullptr;
        LevelExpAndDebt      m_experience_and_debt_per_level;

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
        bool            read_fx(const QString &directory_path);
public:
                        GameDataStore();
                        ~GameDataStore();
        bool            read_game_data(const QString &directory_path);
        const ColorAndPartPacker *getPacker() const { return packer_instance; }
        uint32_t        expForLevel(uint32_t lev) const;
        uint32_t        expDebtForLevel(uint32_t lev) const;
        uint32_t        expMaxLevel() const;
        int             countForLevel(uint32_t lvl,const std::vector<uint32_t> &schedule) const;
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
        std::vector<struct FxInfo>  m_fx_infos;
        float                       m_player_fade_in;

        // keep in mind the hierarchy is all_powers -> powercat -> powerset -> powerdata (template)
        const StoredPowerCategory&  get_power_category(uint32_t pcat_idx);
        const Parse_PowerSet&       get_powerset(uint32_t pcat_idx, uint32_t pset_idx);
        const Power_Data&           get_power_template(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);
        Power_Data*                 editable_power_tpl(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);

        // auto-AFK and logout settings, auto-AFK is mandatory, server can choose between auto-logout or not
        float                       m_time_to_afk = 5 * 60;     // default afk time is 5 mins (300 secs)
        // when player has reached the threshold for m_time_to_logout, they will be notified that they will be
        // auto-logged out in m_time_to_auto_logout seconds
        // so m_time_to_logout_msg + m_time_to_auto_logout is when the player will be kicked out of the server
        float                       m_time_to_logout_msg = 18 * 60;
        float                       m_time_to_auto_logout = 2 * 60;
        bool                        m_uses_auto_logout = true;
};
int getEntityOriginIndex(const GameDataStore &data,bool is_player, const QString &origin_name);
int getEntityClassIndex(const GameDataStore &data,bool is_player, const QString &class_name);
extern GameDataStore& getGameData();
