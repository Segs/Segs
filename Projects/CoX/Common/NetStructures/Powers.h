/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"
#include <QtCore/QString>
#include "BitStream.h"

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
    int category_idx;
    int powerset_entry_idx;
    int power_idx;
    void serializefrom( BitStream &src );
    void serializeto( BitStream &src ) const;
};

class Power
{
public:
    TrayItemType entry_type=TrayItemType(0);
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
    Power m_powers[10];
    Power *getPower(size_t idx);
    int setPowers();
    void serializefrom(BitStream &src);
    void serializeto(BitStream &tgt) const;
    void Dump();
};
class PowerTrayGroup
{
    static const int num_trays=2; // was 3, displayed trays
    PowerTray m_trays[9];
    uint32_t m_default_powerset_idx,m_default_power_idx;
    bool m_c;
    int primary_tray_idx=0;
    int secondary_tray_idx=1;
public:
    PowerTrayGroup()
    {
        m_default_powerset_idx=m_default_power_idx=0;
        m_c=false;
    }
    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void dump();

};
