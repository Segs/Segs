/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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

class CharacterCostume;
struct PlayerData;
struct Costume;

struct CharacterPowerBoost
{
    PowerPool_Info boost_id;
    int            level        = 0;
    int            num_combines = 0;
};

struct CharacterPower
{
    PowerPool_Info                   power_id;
    int                              bought_at_level = 0;
    float                            range           = 1.0f;
    std::vector<CharacterPowerBoost> boosts;
};

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

        using vPowerPool = std::vector<CharacterPower>;
        using vCostume = std::vector<CharacterCostume>;

        vPowerPool              m_powers;
        PowerTrayGroup          m_trays;
        uint64_t                m_owner_account_id;
        uint8_t                 m_player_collisions=0;
        friend bool toActualCharacter(const struct GameAccountResponseCharacterData &src, Character &tgt,PlayerData &player, EntityData &entity);
        friend bool fromActualCharacter(const Character &src,const PlayerData &player, const EntityData &entity, GameAccountResponseCharacterData &tgt);
public:
                        Character();
//////////////////////////////////////////////////////////////////////////
// Getters and setters
const   QString &       getName() const { return m_name; }
        void            setName(const QString &val);
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
        void            serializetoCharsel(BitStream &bs);
        void            GetCharBuildInfo(BitStream &src); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src, const ColorAndPartPacker *packer);
        const CharacterCostume *getCurrentCostume() const;
        void            DumpSidekickInfo();
        void            DumpPowerPoolInfo( const PowerPool_Info &pool_info );
        void            DumpBuildInfo();
        void            face_bits(uint32_t){}
        void            dump();
        void            sendFullStats(BitStream &bs) const;
        void            sendTray(BitStream &bs) const;
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
        PowerPool_Info  get_power_info(BitStream &src);
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
bool toActualCharacter(const GameAccountResponseCharacterData &src, Character &tgt, PlayerData &player, EntityData &entity);
bool fromActualCharacter(const Character &src, const PlayerData &player, const EntityData &entity, GameAccountResponseCharacterData &tgt);
