/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include <QtCore/QString>
#include "BitStream.h"

#include <array>

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


class PowerPool_Info
{
public:
    static const constexpr  uint32_t    class_version   = 1;
    int category_idx;
    int powerset_idx;
    int power_idx;
    void serializefrom( BitStream &src );
    void serializeto( BitStream &src ) const;
};

struct CharacterPowerBoost
{
    static const constexpr  uint32_t    class_version   = 1;
    PowerPool_Info boost_id;
    int            level        = 0;
    int            num_combines = 0;
};

struct CharacterPower
{
    static const constexpr  uint32_t    class_version   = 1;
    PowerPool_Info                   power_id;
    int                              bought_at_level = 0;
    float                            range           = 1.0f;
    std::vector<CharacterPowerBoost> boosts;
};

struct CharacterPowerSet
{
    static const constexpr  uint32_t    class_version   = 1;
    int                             m_level_bought = 0;
    std::vector<CharacterPower>     m_powers;
};

using vPowerSets = std::vector<CharacterPowerSet>;

class Power
{
public:
    static const constexpr  uint32_t    class_version   = 1;
    TrayItemType entry_type = TrayItemType(0);
    int powerset_idx,power_idx;
    QString command;
    QString short_name;
    QString icon_name;
    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);

    void Dump();
};

class PowerTray
{
public:
    static const constexpr  uint32_t    class_version   = 1;
    std::array<Power, 10>     m_powers;
    Power *getPower(size_t idx);
    int setPowers();
    void serializefrom(BitStream &src);
    void serializeto(BitStream &tgt) const;
    void Dump();
};

class PowerTrayGroup
{
public:
    static const constexpr  uint32_t    class_version   = 1;
    static const int num_trays=2; // was 3, displayed trays
    std::array<PowerTray, 9>     m_trays;
    uint32_t m_default_powerset_idx,m_default_power_idx;
    bool m_has_default_power;
    int primary_tray_idx=0;
    int secondary_tray_idx=1;
    PowerTrayGroup()
    {
        m_default_powerset_idx=m_default_power_idx=0;
        m_has_default_power=false;
    }
    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void dump();

};
