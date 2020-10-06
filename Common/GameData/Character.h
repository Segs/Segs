/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "Components/BitStream.h"
#include "Powers.h"
#include "Costume.h"
#include "Common/GameData/CharacterData.h"
#include "Common/GameData/CharacterAttributes.h"
#include "Common/GameData/entitydata_definitions.h"
#include "Common/GameData/clientoptions_definitions.h"
#include "Common/GameData/gui_definitions.h"
#include "Common/GameData/keybind_definitions.h"
#include "Common/GameData/other_definitions.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <cassert>
#include <string>

namespace SEGSEvents
{
    struct GameAccountResponseCharacterData;
}

struct PlayerData;
class PowerPool_Info;
class GameDataStore;

enum NameFlag : bool
{
    NoName = false,
    HasName = true,
};

enum ConditionalFlag : bool
{
    Unconditional = false,
    Conditional = true,
};

enum ClientWindowState
{
    None = 0,
    Training = 1,
    Tailor = 2,
    Supergroup = 3,
    Enhancements = 4,
    Id = 5,
    AwaitingGurney = 6

};

class Character
{
        friend  class CharacterDatabase;

        uint64_t                m_owner_account_id;
        uint8_t                 m_player_collisions=0;
        friend bool toActualCharacter(const SEGSEvents::GameAccountResponseCharacterData &src, Character &tgt,PlayerData &player, EntityData &entity);
        friend bool fromActualCharacter(const Character &src, const PlayerData &player, const EntityData &entity, SEGSEvents::GameAccountResponseCharacterData &tgt);
public:
                        Character();
//////////////////////////////////////////////////////////////////////////
// Getters and setters
const   QString &       getName() const { return m_name; }
        void            setName(const QString &val);
        float           getHealth() { return m_char_data.m_current_attribs.m_HitPoints; }
        void            setHealth(float val) { m_char_data.m_current_attribs.m_HitPoints = std::max(0.0f, std::min(val, m_max_attribs.m_HitPoints)); }
        uint8_t         getIndex() const { return m_index; }
        void            setIndex(uint8_t val) { m_index = val; }
        uint64_t        getAccountId() const { return m_owner_account_id; }
        void            setAccountId(uint64_t val) { m_owner_account_id = val; }


//
//////////////////////////////////////////////////////////////////////////
        void            reset();
        bool            isEmpty();
        void            serializefrom(BitStream &buffer);
        void            serializeto(BitStream &buffer) const;
        void            serialize_costumes(BitStream &buffer, const ColorAndPartPacker *packer, bool send_all_costumes=true) const;
        void            serializetoCharsel(BitStream &bs, const QString& entity_map_name);
        void            finalizeLevel();
        void            addStartingInspirations(QStringList &starting_insps);
        void            addStartingPowers(const QString &pcat_name, const QString &pset_name, const QStringList &power_names);
        void            addPowersByLevel(const QString &pcat_name, const QString &pset_name, uint32_t level);
        void            getPowerFromBuildInfo(BitStream &src);
        void            finalizeCombatLevel();
        void            sendEnhancements(BitStream &bs) const;
        void            sendInspirations(BitStream &bs) const;
        void            GetCharBuildInfo(BitStream &src); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src, const ColorAndPartPacker *packer);
        const Costume   *getCurrentCostume() const;
        const vCostumes *getAllCostumes() const;
        void            addCostumeSlot();
        void            saveCostume(uint32_t idx, Costume &new_costume);
        void            dumpSidekickInfo();
        void            dumpBuildInfo();
        void            face_bits(uint32_t){}
        void            dump();
        void            sendFullStats(BitStream &bs) const;
        void            sendTray(BitStream &bs) const;
        void            sendOwnedPowers(BitStream &bs) const;
        void            sendDescription(BitStream &bs) const;
        void            sendTitles(BitStream &bs, NameFlag hasname, ConditionalFlag conditional) const;
        void            sendFriendList(BitStream &bs) const;

        // Parse_CharAttrib    m_current_attribs; --> placed in CharacterData now
        Parse_CharAttrib    m_max_attribs;
        LevelExpAndDebt     m_other_attribs;
        CharacterData       m_char_data;

        uint32_t            m_account_id;
        uint32_t            m_db_id;
        ClientWindowState   m_client_window_state = ClientWindowState::None;
        bool                m_add_new_costume = true;
        bool                m_is_dead = false;

protected:
        QString         m_name;
        vCostumes       m_costumes;
        Costume *       m_sg_costume;
        uint8_t         m_index;
        enum CharBodyType
        {
            TYPE_MALE,
            TYPE_FEMALE,
            TYPE_UNUSED1,
            TYPE_UNUSED2,
            TYPE_HUGE,
            TYPE_NOARMS
        };
};

void serializeStats(const Character &src, BitStream &bs, bool sendAbsolute);
bool initializeCharacterFromCreator();
bool toActualCharacter(const SEGSEvents::GameAccountResponseCharacterData &src, Character &tgt, PlayerData &player, EntityData &entity);
bool fromActualCharacter(const Character &src, const PlayerData &player, const EntityData &entity, SEGSEvents::GameAccountResponseCharacterData &tgt);
