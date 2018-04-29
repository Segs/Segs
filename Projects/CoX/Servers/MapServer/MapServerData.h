/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "Common/GameData/CoXHash.h"
#include "Common/GameData/costume_definitions.h"
#include "Common/GameData/origin_definitions.h"
#include "Common/GameData/charclass_definitions.h"
#include "Common/GameData/keybind_definitions.h"
#include "Common/GameData/other_definitions.h"
#include "Common/GameData/npc_definitions.h"

#include "NpcStore.h"

class ColorAndPartPacker;
class QString;

class MapServerData
{
        ColorAndPartPacker *packer_instance;
        LevelExpAndDebt m_experience_and_debt_per_level;

        bool            read_costumes(const QString &directory_path);
        bool            read_colors(const QString &src_filename);
        bool            read_origins(const QString &src_filename);
        bool            read_classes(const QString &src_filename);
        bool            read_exp_and_debt(const QString &src_filename);
        bool            read_keybinds(const QString &src_filename);
        bool            read_commands(const QString &src_filename);
        bool            read_npcs(const QString &directory_path);
public:
                        MapServerData();
                        ~MapServerData();
        bool            read_runtime_data(const QString &directory_path);
        const ColorAndPartPacker *getPacker() const { return packer_instance; }
        int             expForLevel(int lev) const;
        int             expDebtForLevel(int lev) const;
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
};
