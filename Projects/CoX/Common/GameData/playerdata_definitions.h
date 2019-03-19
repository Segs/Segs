/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "clientoptions_definitions.h"
#include "gui_definitions.h"
#include "keybind_definitions.h"
#include "Common/GameData/Clue.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/CharacterAttributes.h"
#include "Common/GameData/PlayerStatistics.h"
#include "Common/GameData/Task.h"

#include "cereal/types/map.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/bitset.hpp"


#include <stdint.h>
#include <bitset>

struct PlayerProgress
{
    std::map<std::string, std::bitset<1024>>   m_visible_map_tiles;
    std::bitset<1024> m_test;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

template<class Archive>
void PlayerProgress::serialize(Archive &archive, uint32_t const version)
 {
     archive(cereal::make_nvp("VisibleMapTiles", m_test));
 };

// this part of the Entity is for Players only info
struct PlayerData
{
    enum : uint32_t {class_version   = 3};
    GUISettings      m_gui;
    KeybindSettings  m_keybinds;
    ClientOptions    m_options;
    vContactList     m_contacts;
    vTaskEntryList   m_tasks_entry_list;
    vClueList        m_clues;
    vSouvenirList    m_souvenirs;
    PlayerStatistics m_player_statistics;
    PlayerProgress   m_player_progress;

    uint8_t             m_auth_data[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; // preorder rewards and things

    void reset()
    {
        m_options.m_first_person_view=false;
    }
    void dump() const
    {
        m_options.clientOptionsDump();
    }

};
