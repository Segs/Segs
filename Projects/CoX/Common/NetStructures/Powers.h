/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "BitStream.h"
#include "GameData/power_definitions.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <array>

class Entity;
struct CharacterData;

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

struct EnhancemenSlotEntry
{
    bool        m_set_in_power  = false;
    uint32_t    m_pset_idx      = 0;
    uint32_t    m_pow_idx       = 0;
    uint32_t    m_eh_idx        = 0;
};

class PowerPool_Info
{
public:
static const constexpr  uint32_t    class_version = 1;
        uint32_t        m_pcat_idx     = 0;
        uint32_t        m_pset_idx         = 0;
        uint32_t        m_pow_idx          = 0;
        void serializefrom( BitStream &src );
        void serializeto( BitStream &src ) const;
};

struct CharacterInspiration
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_insp_info;
        Power_Data      m_insp_tpl;
        QString         m_name;
        uint32_t        m_col               = 0;
        uint32_t        m_row               = 0;
        bool            m_has_insp          = false;
};

struct vInspirations
{
    std::vector<CharacterInspiration> m_inspirations;

    // although in some cases people prefer [row][col], i'll be using x -> col and y -> row
    size_t m_rows, m_cols;

    vInspirations()
    {
        m_cols = 5;
        m_rows = 4;
        // m_inspirations.reserve(m_rows * m_cols);
    }

    vInspirations(size_t cols, size_t rows)
    {
        if (cols > 5)
            qCritical() << "vInspirations has more than 5 columns!";
        if (rows > 4)
            qCritical() << "vInspirations has more than 4 rows!";

        m_cols = cols;
        m_rows = rows;
        // m_inspirations.reserve(m_cols * m_rows);
    }

    const size_t size()
    {
        return m_inspirations.size();
    }

    CharacterInspiration& at (size_t col, size_t row) const
    {
        // Note: possible arrayIndexOutOfRange exception here
        return m_inspirations[col * row];
    }
};

struct CharacterEnhancement
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_enhance_info;
        Power_Data      m_enhance_tpl;
        uint32_t        m_slot_idx          = 0;
        QString         m_name;
        uint32_t        m_level             = 0;
        uint32_t        m_num_combines      = 0;
        bool            m_slot_used         = false;
};

using vEnhancements = std::array<CharacterEnhancement, 10>;

struct CharacterPower
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_power_info;
        Power_Data      m_power_tpl;
        uint32_t        m_index             = 0;
        QString         m_name;
        uint32_t        m_level_bought      = 0;
        uint32_t        m_num_charges       = 0;
        float           m_usage_time        = 0.0f;
        uint32_t        m_activation_time   = 0;    // seconds since Jan 1, 2000
        float           m_range             = 1.0f;
        float           m_recharge_time     = 0.0f;
        uint32_t        m_activation_state  = 0;
        uint32_t        m_total_eh_slots    = 0;
        bool            m_active_state_change   = false;
        bool            m_timer_updated         = false;
        bool            m_erase_power           = false;
        std::array<CharacterEnhancement, 6> m_enhancements;
};

struct CharacterPowerSet
{
static const constexpr  uint32_t    class_version   = 1;
        uint32_t                    m_index         = 0;
        uint32_t                    m_category      = 0;
        uint32_t                    m_level_bought  = 0;
        std::vector<CharacterPower> m_powers;
};

using vPowerSets = std::vector<CharacterPowerSet>;

class PowerTrayItem
{
public:
static const constexpr  uint32_t    class_version = 1;
    TrayItemType    m_entry_type    = TrayItemType(0);
    uint32_t        m_pset_idx      = 0;
    uint32_t        m_pow_idx       = 0;
    QString         m_command;
    QString         m_short_name;
    QString         m_icon_name;

    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void Dump();
};

class PowerTray
{
public:
static const constexpr  uint32_t    class_version = 1;
    std::array<PowerTrayItem, 10>     m_tray_items;
    PowerTrayItem *getPowerTrayItem(size_t idx);
    int setPowers();
    void serializefrom(BitStream &src);
    void serializeto(BitStream &tgt) const;
    void Dump();
};

class PowerTrayGroup
{
public:
static const constexpr  uint32_t    class_version = 1;
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

};


/*
 * Powers Methods
 */
int     getPowerCatByName(const QString &name);
int     getPowerSetByName(const QString &name, uint32_t pcat_idx);
int     getPowerByName(const QString &name, uint32_t pcat_idx, uint32_t pset_idx);
CharacterPower getPowerData(PowerPool_Info &ppool);
CharacterPowerSet getPowerSetData(PowerPool_Info &ppool);
CharacterPower * getOwnedPower(Entity &e, uint32_t pset_idx, uint32_t pow_idx);
void addPowerSet(CharacterData &cd, PowerPool_Info &ppool);
void addEntirePowerSet(CharacterData &cd, PowerPool_Info &ppool);
void addPower(CharacterData &cd, PowerPool_Info &ppool);
void removePower(CharacterData &cd, const PowerPool_Info &ppool);
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t tgt_idx, uint32_t tgt_id);
void dumpPowerPoolInfo(const PowerPool_Info &pinfo);
void dumpPower(const CharacterPower &pow);
void dumpOwnedPowers(CharacterData &cd);


/*
 * Inspirations Methods
 */
void addInspirationByName(CharacterData &cd, QString &name);
void addInspirationToChar(CharacterData &cd, CharacterInspiration insp);
void moveInspiration(CharacterData &cd, uint32_t src_col, uint32_t src_row, uint32_t dest_col, uint32_t dest_row);
void useInspiration(Entity &ent, uint32_t col, uint32_t row);
void removeInspiration(CharacterData &cd, uint32_t col, uint32_t row);
void dumpInspirations(CharacterData &cd);


/*
 * Enhancements Methods
 */
void addEnhancementByName(CharacterData &cd, QString &name, uint32_t &level);
CharacterEnhancement *getSetEnhancementBySlot(Entity &e, uint32_t pset_idx_in_array, uint32_t pow_idx_in_array, uint32_t eh_slot);
int getNumberEnhancements(CharacterData &cd);
void moveEnhancement(CharacterData &cd, uint32_t src_idx, uint32_t dest_idx);
void setEnhancement(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t src_idx, uint32_t dest_idx);
void trashEnhancement(CharacterData &cd, uint32_t eh_idx);
void trashEnhancementInPower(CharacterData &cd, uint32_t pset_idx, uint32_t pow_idx, uint32_t eh_idx);
void trashComboEnhancement(CharacterEnhancement &eh, uint32_t eh_idx);
void buyEnhancementSlot(Entity &e, uint32_t num, uint32_t pset_idx, uint32_t pow_idx);
void reserveEnhancementSlot(CharacterData &cd, CharacterPower *pow);
void combineEnhancements(Entity &ent, EnhancemenSlotEntry slot1, EnhancemenSlotEntry slot2);
void dumpEnhancements(CharacterData &cd);
