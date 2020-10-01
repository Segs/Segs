/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
#include "Common/GameData/seq_definitions.h"
#include "Common/GameData/shop_definitions.h"
#include "Common/GameData/bodypart_definitions.h"

#include "NpcStore.h"
#include <QDate>
#include <QHash>

class ColorAndPartPacker;
class IndexedStringPacker;
class QString;
struct FxInfo;
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
        bool            read_sequencer_definitions(const QString &directory_path);
        bool            read_store_data(const QString &directory_path);
        bool            read_store_items_data(const QString &directory_path);
        bool            read_store_depts_data(const QString &directory_path);
        bool            read_sequencer_types(const QString &directory_path);
        bool            read_body_parts(const QString &directory_path);
public:
                        GameDataStore();
                        ~GameDataStore();
        bool            read_game_data(const QString &directory_path);
        const ColorAndPartPacker *getPacker() const { return packer_instance; }
        uint32_t        expForLevel(uint32_t lev) const;
        uint32_t        expDebtForLevel(uint32_t lev) const;
        uint32_t        expMaxLevel() const;
        uint32_t countForLevel(uint32_t lvl,const std::vector<uint32_t> &schedule) const;
        const NPCStorage & getNPCDefinitions() const
                        {
                            return m_npc_store;
                        }
        FxInfo *        getFxInfoByName(const QByteArray &name);

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
        std::vector<FxInfo>         m_fx_infos;
        AllShops_Data               m_shops_data;
        AllShopItems_Data           m_shop_items_data;
        AllShopDepts_Data           m_shop_depts_data;
        float                       m_player_fade_in;
        float                       m_motd_timer = 60 * 60; // default 1 hr
        QStringList                 m_costume_slot_unlocks; // used in finalizeLevel() to award costume slots
        SequencerList               m_seq_definitions; // animation sequencer definitions
        SequencerTypeMap            m_seq_types;
        BodyPartsStorage            m_body_parts;

        // keep in mind the hierarchy is all_powers -> powercat -> powerset -> powerdata (template)
        const StoredPowerCategory&  get_power_category(uint32_t pcat_idx);
        const Parse_PowerSet&       get_powerset(uint32_t pcat_idx, uint32_t pset_idx);
        const Power_Data&           get_power_template(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);
        Power_Data*                 editable_power_tpl(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);
        int                         getFxNamePackId(const QString &name);

        // auto-AFK and logout settings, auto-AFK is mandatory, server can choose between auto-logout or not
        float                       m_time_to_afk = 5 * 60;     // default afk time is 5 mins (300 secs)
        // when player has reached the threshold for m_time_to_logout, they will be notified that they will be
        // auto-logged out in m_time_to_auto_logout seconds
        // so m_time_to_logout_msg + m_time_to_auto_logout is when the player will be kicked out of the server
        float                       m_time_to_logout_msg = 18 * 60;
        float                       m_time_to_auto_logout = 2 * 60;
        bool                        m_uses_auto_logout = true;

        // experience modifier settings
        bool                        m_uses_xp_mod;
        double                      m_xp_mod_multiplier;
        QDateTime                   m_xp_mod_startdate;
        QDateTime                   m_xp_mod_enddate;

        // default of 30 for cases where settings are not yet loaded
        int                         m_world_update_ticks_per_sec=30;
private:
        // Helper structs
        QHash<QByteArray,int>       m_name_to_fx_index;
};
int getEntityOriginIndex(const GameDataStore &data,bool is_player, const QString &origin_name);
int getEntityClassIndex(const GameDataStore &data,bool is_player, const QString &class_name);
extern GameDataStore& getGameData();
