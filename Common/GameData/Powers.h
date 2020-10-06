/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/BitStream.h"
#include "Common/GameData/CommonNetStructures.h"
#include "Common/GameData/power_definitions.h"
#include "Common/GameData/GameDataStore.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <array>

class Entity;
struct CharacterData;
class GameDataStore;

enum class TrayItemType : uint32_t
{
    None                    = 0,
    Power                   = 1,
    Inspiration             = 2,
    BodyItem                = 3,
    SpecializationPower     = 4,
    SpecializationInventory = 5,
    Macro                   = 6,
    Count                   = 7,
};

enum class ReviveLevel
{
    AWAKEN              = 1,
    BOUNCE_BACK         = 2,
    RESTORATION         = 3,
    IMMORTAL_RECOVERY   = 4,
    REGEN_REVIVE        = 5,
    FULL                = 6,
};

struct PowerStance
{
    bool        has_stance  = false;
    uint32_t    pset_idx    = 0;
    uint32_t    pow_idx     = 0;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(has_stance);
        ar(pset_idx);
        ar(pow_idx);
    }
};

struct EnhancemenSlotEntry
{
    bool        m_set_in_power  = false;
    uint32_t    m_pset_idx      = 0;
    uint32_t    m_pow_idx       = 0;
    uint32_t    m_eh_idx        = 0;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_set_in_power, m_pset_idx, m_pow_idx, m_eh_idx);
    }
};

struct PowerVecIndexes
{
    uint32_t        m_pset_vec_idx      = 0;
    uint32_t        m_pow_vec_idx       = 0;
};

struct QueuedPowers
{
    PowerVecIndexes m_pow_idxs;
    float           m_activate_period       = 0.0f;
    float           m_recharge_time         = 0.0f;
    float           m_time_to_activate      = 0.0f;
    bool            m_active_state_change   = false;
    bool            m_activation_state      = false;
    bool            m_timer_updated         = false;
    uint32_t        m_tgt_idx               = 0;
};

class PowerPool_Info
{
public:
enum : uint32_t { class_version = 1 };
        uint32_t        m_pcat_idx          = 0;
        uint32_t        m_pset_idx          = 0;
        uint32_t        m_pow_idx           = 0;

        void serializefrom( BitStream &src );
        void serializeto( BitStream &src ) const;
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

struct buffset
{
    float           m_value                 = 0.0;
    QString         m_value_name            = "";
    float           m_duration              = 0.0f;
    uint32_t        m_attrib                = 0;       // for damage resistances or defenses or others
};

struct Buffs
{
    QString         m_name                  = "unknown";
    PowerPool_Info  m_buff_info;                        //There is one buff for each power, so that only one icon is shown
    std::vector<buffset>    m_buffs;                    //powers with multiple effects have a buffset per effect
    uint32_t         source_ent_idx          = 0;
};

struct CharacterInspiration
{
        enum : uint32_t {class_version = 2};
        PowerPool_Info  m_insp_info;
        Power_Data      m_insp_tpl;
        QString         m_name;
        uint32_t        m_col               = 0;
        uint32_t        m_row               = 0;
        bool            m_has_insp          = false;
        
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

struct vInspirations
{
    enum : uint32_t {class_version = 1};
    
    std::vector<std::vector<CharacterInspiration>> m_inspirations;

    // although in some cases people prefer [row][col], i'll be using x -> col and y -> row
    size_t m_rows, m_cols;

    vInspirations(size_t cols = 5, size_t rows = 4)
    {
        if(cols > 5)
            qCritical() << "vInspirations has more than 5 columns!";
        if(rows > 4)
            qCritical() << "vInspirations has more than 4 rows!";

        m_cols = cols;
        m_rows = rows;
        // m_inspirations.reserve(m_cols * m_rows);

        // first, create the columns
        m_inspirations.resize(m_cols);

        // then in each column, create the rows
        for (size_t i = 0; i < m_cols; ++i)
            m_inspirations[i].resize(m_rows);
    }

    uint32_t size()
    {
        return m_cols * m_rows;
    }

    CharacterInspiration& at (const size_t col, const size_t row)
    {
        if(col >= m_cols || row >= m_rows)
            qCritical() << QString("Trying to access vInspirations of %1 rows %2 cols using params %3 rows %4 cols");

        return m_inspirations[col][row];
    }

    const CharacterInspiration& at (const size_t col, const size_t row) const
    {
        if(col >= m_cols || row >= m_rows)
            qCritical() << QString("Trying to access vInspirations of %1 rows %2 cols using params %3 rows %4 cols");

        return m_inspirations[col][row];
    }

    CharacterInspiration& from_first_row (const size_t col)
    {
        if(!m_inspirations[col][0].m_has_insp)
            push_to_first_row(col);

        return m_inspirations[col][0];
    }

    CharacterInspiration value(const size_t col, const size_t row) const
    {
        return m_inspirations[col][row];
    }

    void resize (const size_t newCol, const size_t newRow)
    {
        m_inspirations.resize(newCol);
        for (size_t i = 0; i < m_cols; ++i)
            m_inspirations[i].resize(newRow);

        m_cols = newCol;
        m_rows = newRow;
    }

    void push_to_first_row (const size_t col)
    {
        size_t i = 1;
        while (m_inspirations[col][0].m_has_insp && i < m_rows)
        {
            if(m_inspirations[col][i].m_has_insp)
                std::swap(m_inspirations[col][0], m_inspirations[col][i]);
            i++;
        }
    }
    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

struct CharacterEnhancement
{
        enum : uint32_t {class_version = 1};
        PowerPool_Info  m_enhance_info;
        Power_Data      m_enhance_tpl;
        uint32_t        m_slot_idx          = 0;
        QString         m_name;
        uint32_t        m_level             = 0;
        uint32_t        m_num_combines      = 0;
        bool            m_slot_used         = false;
        
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

using vEnhancements = std::array<CharacterEnhancement, 10>;

struct CharacterPower
{
        enum : uint32_t {class_version = 2}; // v2: is_limited and charges
        PowerPool_Info  m_power_info;
        uint32_t        m_index             = 0;
        uint32_t        m_level_bought      = 0;
        uint32_t        m_total_eh_slots    = 0;
        uint32_t        m_charges_remaining = 0;

        // Timers and Flags
        uint32_t        m_activate_period       = 0;     // casting time
        float           m_usage_time            = 0.0f;  // total time you can use toggle power
        float           m_lifetime              = 0.0f;  // lifetime for temp powers
        bool            m_is_limited            = false; // some temporary powers have charges or a time limit
        bool            m_active_state_change   = false;
        bool            m_activation_state      = false;
        bool            m_timer_updated         = false;
        bool            m_erase_power           = false;

        std::vector<CharacterEnhancement> m_enhancements; // max of 5 enhancement slots for this client version

        Power_Data getPowerTemplate() const;
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

struct CharacterPowerSet
{
        enum : uint32_t {class_version   = 1};
        uint32_t                    m_index         = 0;
        uint32_t                    m_category      = 0;
        uint32_t                    m_level_bought  = 0;
        std::vector<CharacterPower> m_powers;
        
        template<class Archive>
        void serialize(Archive &archive, uint32_t const version);
};

using vPowerSets = std::vector<CharacterPowerSet>;

class PowerTrayItem
{
public:
enum : uint32_t { class_version=1 };
    TrayItemType    m_entry_type    = TrayItemType(0);
    uint32_t        m_pset_idx      = 0;
    uint32_t        m_pow_idx       = 0;
    QString         m_command;
    QString         m_short_name;
    QString         m_icon_name;

    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void Dump();

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

class PowerTray
{
public:
enum : uint32_t { class_version=1 };
    std::array<PowerTrayItem, 10>     m_tray_items;
    PowerTrayItem *getPowerTrayItem(size_t idx);
    int setPowers();

    void serializefrom(BitStream &src);
    void serializeto(BitStream &tgt) const;
    void Dump();

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};

class PowerTrayGroup
{
public:
enum : uint32_t { class_version=1 };
static const int m_num_trays = 2; // was 3, displayed trays
    std::array<PowerTray, 9>     m_trays;
    uint32_t m_default_pset_idx = 0;
    uint32_t m_default_pow_idx  = 0;
    bool m_has_default_power    = false;
    int m_primary_tray_idx      = 0;
    int m_second_tray_idx       = 1;

    PowerTrayGroup()
    {
        m_default_pset_idx = m_default_pow_idx = 0;
        m_has_default_power = false;
    }

    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void dump();

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};
struct DelayedEffect
{
    StoredAttribMod mod;
    CharacterPower *power;
    int m_timer;
    int ticks;
    uint32_t src_ent;
};

/*
 * Powers Methods
 */
int     getPowerCatByName(const QString &name);
int     getPowerSetByName(const QString &name, uint32_t pcat_idx);
int     getPowerByName(const QString &name, uint32_t pcat_idx, uint32_t pset_idx);
CharacterPower getPowerData(PowerPool_Info &ppool);
CharacterPowerSet getPowerSetData(PowerPool_Info &ppool);
CharacterPower *getOwnedPowerByVecIdx(Entity &e, uint32_t pset_idx, uint32_t pow_idx);
CharacterPower *getOwnedPowerByTpl(Entity &e, const PowerPool_Info &ppool);
PowerVecIndexes getOwnedPowerIndexes(Entity &e, const PowerPool_Info &ppool);
void addPowerSet(CharacterData &cd, PowerPool_Info &ppool);
void addEntirePowerSet(CharacterData &cd, PowerPool_Info &ppool);
void addPower(CharacterData &cd, PowerPool_Info &ppool);
void removePower(CharacterData &cd, const PowerPool_Info &ppool);
uint32_t countAllOwnedPowers(CharacterData &cd, bool include_temps);
void dumpPowerPoolInfo(const PowerPool_Info &pinfo);
void dumpPower(const CharacterPower &pow);
void dumpOwnedPowers(CharacterData &cd);

/*
 * Inspirations Methods
 */
const CharacterInspiration* getInspiration(const Entity &ent, uint32_t col, uint32_t row);
void addInspirationByName(CharacterData &cd, QString &name);
void addInspirationToChar(CharacterData &cd, const CharacterInspiration& insp);
int getNumberInspirations(const CharacterData &cd);
int getMaxNumberInspirations(const CharacterData &cd);
void moveInspiration(CharacterData &cd, uint32_t src_col, uint32_t src_row, uint32_t dest_col, uint32_t dest_row);
void removeInspiration(CharacterData &cd, uint32_t col, uint32_t row);
void dumpInspirations(CharacterData &cd);


/*
 * Enhancements Methods
 */
void addEnhancementByName(CharacterData &cd, QString &name, uint32_t &level);
void addEnhancementToChar(CharacterData &cd, const CharacterEnhancement& enh);
CharacterEnhancement *getSetEnhancementBySlot(Entity &e, uint32_t pset_idx_in_array, uint32_t pow_idx_in_array, uint32_t eh_slot);
const CharacterEnhancement* getEnhancement(const Entity &ent, uint32_t idx);
uint32_t getNumberEnhancements(const CharacterData& cd);
uint32_t getMaxNumberEnhancements(const CharacterData &cd);
void moveEnhancement(CharacterData &cd, uint32_t src_idx, uint32_t dest_idx);
void setEnhancement(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t src_idx, uint32_t dest_idx);
void trashEnhancement(CharacterData &cd, uint32_t eh_idx);
void trashEnhancementInPower(CharacterData &cd, uint32_t pset_idx, uint32_t pow_idx, uint32_t eh_idx);
void trashComboEnhancement(CharacterEnhancement &eh, uint32_t eh_idx);
void reserveEnhancementSlot(CharacterPower *pow, uint32_t level_purchased);
void buyEnhancementSlots(Entity &ent, uint32_t available_slots, std::vector<int> pset_idx, std::vector<int> pow_idx);


struct CombineResult
{
    bool success;
    bool destroyed;
};

CombineResult combineEnhancements(Entity &ent, const EnhancemenSlotEntry &slot1, const EnhancemenSlotEntry &slot2);
void dumpEnhancements(CharacterData &cd);
