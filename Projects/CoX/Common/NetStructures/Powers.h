/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "BitStream.h"

#include <QtCore/QString>
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

struct PreorderSprint
{
        bool    m_has_preorder = true;
        QString m_name;
};

class PowerPool_Info
{
public:
static const constexpr  uint32_t    class_version = 1;
        uint32_t        m_category_idx     = 0;
        uint32_t        m_pset_idx         = 0;
        uint32_t        m_pow_idx          = 0;
        void serializefrom( BitStream &src );
        void serializeto( BitStream &src ) const;
};

struct CharacterInspiration
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_insp_tpl;
        QString         m_name;
        uint32_t        m_col               = 0;
        uint32_t        m_row               = 0;
        bool            m_has_insp          = false;
};

using vInspirations = std::array< std::array<CharacterInspiration, 4>, 5>;

struct CharacterEnhancement
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_enhance_tpl;
        uint32_t        m_enhancement_idx   = 0;
        QString         m_name;
        uint32_t        m_level             = 0;
        uint32_t        m_num_combines      = 0;
        bool            m_slot_used         = false;
};

using vEnhancements = std::array<CharacterEnhancement, 10>;

struct CharacterPower
{
static const constexpr  uint32_t    class_version = 1;
        PowerPool_Info  m_power_tpl;
        uint32_t        m_index             = 0;
        QString         m_name;
        uint32_t        m_level_bought      = 0;
        uint32_t        m_num_charges       = 0;
        float           m_usage_time        = 0.0f;
        uint32_t        m_activation_time   = 0;    // seconds since Jan 1, 2000
        float           m_range             = 1.0f;
        float           m_recharge_time     = 0.0f;
        uint32_t        m_activation_state  = 0;
        uint32_t        m_available_eh_slots = 3;
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
CharacterPower getPowerData(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);
CharacterPowerSet getPowerSetData(uint32_t pcat_idx, uint32_t pset_idx);
CharacterPower * getOwnedPower(Entity &e, uint32_t pset_idx, uint32_t pow_idx);
void addPower(CharacterData &cd, uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx);
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
int getNumberEnhancements(CharacterData &cd);
void moveEnhancement(CharacterData &cd, uint32_t src_idx, uint32_t dest_idx);
void setEnhancement(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t src_idx, uint32_t dest_idx);
void trashEnhancement(CharacterData &cd, uint32_t eh_idx);
void dumpEnhancements(CharacterData &cd);
