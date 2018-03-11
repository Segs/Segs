/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"
#include "BitStream.h"
#include "Powers.h"
#include "Common/GameData/attrib_definitions.h"
#include "Common/GameData/chardata_definitions.h"
#include "Common/GameData/clientoptions_definitions.h"
#include "Common/GameData/other_definitions.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <cassert>
#include <string>

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

// TODO: Move to window definitions file
enum WindowIDX : uint32_t {
    wdw_Unknown0        = 0,
    wdw_Status          = 1,
    wdw_Target          = 2,
    wdw_PowersTray      = 3,
    wdw_Chat            = 4,
    wdw_PowerList       = 5,
    wdw_Team            = 6,
    wdw_NavCompass      = 7,
    wdw_Map             = 8,
    wdw_ChatOptions     = 9,
    wdw_Friends         = 10,
    wdw_Unknown11       = 11,
    wdw_Inspirations    = 12,
    wdw_SuperGroup      = 13,
    wdw_Emails          = 14,
    wdw_ComposeEmail    = 15,
    wdw_Contacts        = 16,
    wdw_Missions        = 17,
    wdw_Clues           = 18,
    wdw_Unknown19       = 19,
    wdw_Quit            = 20,
    wdw_Info            = 21,
    wdw_Help            = 22,
    wdw_Unknown23       = 23,
    wdw_Actions         = 24,
    wdw_Unknown25       = 25,
    wdw_Unknown26       = 26,
    wdw_Unknown27       = 27,
    wdw_GenericDlg      = 28,
    wdw_Unknown29       = 29,
    wdw_Support         = 30,
    wdw_Unknown31       = 31,
    wdw_Defeated        = 32,
    wdw_Unknown33       = 33,
    wdw_CostumeSelect   = 34,
};
enum WindowVisibility : uint32_t {
    wv_Uninitialized    = 0,
    wv_Growing          = 1,
    wv_Visible          = 2,
    wv_Shrinking        = 3,
    wv_DockedOrHidden   = 4,
};
struct WindowS {
    WindowIDX           idx;
    WindowVisibility    mode;           // field_24/start_shrunk - 2, unless closing, then goes from 2 (visible) to 3 to 4 (hidden)
    bool        draggable_frame;        // field_14
    uint32_t    posx;
    uint32_t    posy;
    uint32_t    width   = 0;
    uint32_t    height  = 0;
    uint32_t    locked;                 // field_18/docked? - 0, 2 (idx 12 = 1, idx 0-4,7 = 0)
    uint32_t    color   = 0x3399FF99;   // 865730457 == 0x3399FF99 (light blue with 90% transparency)
    uint32_t    alpha   = 0x88;         // default 136 (0x88)
};

class Character
{
        friend  class CharacterDatabase;

        using vPowerPool = std::vector<CharacterPower>;
        using vCostume = std::vector<Costume *>;

        vPowerPool              m_powers;
        PowerTrayGroup          m_trays;
        bool                    m_full_options=false;
        uint64_t                m_owner_account_id;
        uint8_t                 m_player_collisions=0;
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
        void            serialize_costumes(BitStream &buffer, ColorAndPartPacker *packer, bool all_costumes=true) const;
        void            serializetoCharsel(BitStream &bs);
        void            GetCharBuildInfo(BitStream &src); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src, ColorAndPartPacker *packer);
        Costume *       getCurrentCostume() const;
        void            DumpPowerPoolInfo( const PowerPool_Info &pool_info );
        void            DumpBuildInfo();
        void            face_bits(uint32_t){}
        void            dump();
        void            sendFullStats(BitStream &bs) const;
        //TODO: move these to some kind of Player info class
        void            sendTray(BitStream &bs) const;
        void            sendTrayMode(BitStream &bs) const;
        void            sendWindows(BitStream &bs) const;
        void            sendWindow(BitStream &bs) const;
        void            sendTeamBuffMode(BitStream &bs) const;
        void            sendDockMode(BitStream &bs) const;
        void            sendChatSettings(BitStream &bs) const;
        void            sendDescription(BitStream &bs) const;
        void            sendTitles(BitStream &bs, NameFlag hasname, ConditionalFlag conditional) const;
        void            sendKeybinds(BitStream &bs) const;
        void            sendFriendList(BitStream &bs) const;
        void            sendOptions( BitStream &bs ) const;
        void            sendOptionsFull(BitStream &bs) const;

        Parse_CharAttrib    m_current_attribs;
        Parse_CharAttrib    m_max_attribs;
        LevelExpAndDebt     m_other_attribs;
        CharacterData       m_char_data;
        ClientOptions       m_options;

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

void                serializeStats(const Character &src, BitStream &bs, bool sendAbsolute);
