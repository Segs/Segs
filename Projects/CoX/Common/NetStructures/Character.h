/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "BitStream.h"
#include "Powers.h"
#include "Common/GameData/attrib_definitions.h"
#include "Common/GameData/chardata_definitions.h"
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
class CharacterCostume;

struct PlayerData;
struct Costume;
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

class Character
{
        friend  class CharacterDatabase;

        using vCostume = std::vector<CharacterCostume>;

        uint64_t                m_owner_account_id;
        uint8_t                 m_player_collisions=0;
        friend bool toActualCharacter(const SEGSEvents::GameAccountResponseCharacterData &src, Character &tgt,PlayerData &player, EntityData &entity);
        friend bool fromActualCharacter(const Character &src,const PlayerData &player, const EntityData &entity, SEGSEvents::GameAccountResponseCharacterData &tgt);
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
        void            serialize_costumes(BitStream &buffer, const ColorAndPartPacker *packer, bool all_costumes=true) const;
        void            serializetoCharsel(BitStream &bs, const QString& entity_map_name);
        void            finalizeLevel(const GameDataStore &data);
        void            addStartingInspirations(const GameDataStore &data);
        void            getStartingPowers(const GameDataStore &data, const QString &pcat_name, const QString &pset_name, const QStringList &power_names);
        void            getPowerFromBuildInfo(const GameDataStore &data,BitStream &src);
        void            sendEnhancements(BitStream &bs) const;
        void            sendInspirations(BitStream &bs) const;
        void            GetCharBuildInfo(BitStream &src, const GameDataStore &data); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src, const ColorAndPartPacker *packer);
        const CharacterCostume *getCurrentCostume() const;
        void            DumpSidekickInfo();
        void            DumpBuildInfo();
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

protected:
        uint8_t         m_index;
        QString         m_name;
        bool            m_villain;
        vCostume        m_costumes;
        Costume *       m_sg_costume;
        uint32_t        m_current_costume_idx;
        bool            m_current_costume_set;
        uint32_t        m_num_costumes;
        bool            m_multiple_costumes;  // has more then 1 costume
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
