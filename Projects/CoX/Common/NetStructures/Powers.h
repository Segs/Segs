/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"
#include <string>
#include "BitStream.h"

class PowerPool_Info
{
public:
    int id[3];
    void serializefrom( BitStream &src );
    void serializeto( BitStream &src ) const;
};

class Power
{
public:
    int entry_type;
    int unkn1,unkn2;
    std::string sunkn1;
    std::string sunkn2;
    std::string sunkn3;
    Power() {
        entry_type = 0;
    }
    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);

    void Dump();
};
class PowerTray
{
public:
    int unkn0;
    int unkn1;
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
    uint32_t m_power_rel1,m_power_rel2;
    bool m_c;
    int primary_tray_idx,secondary_tray_idx;
public:
    PowerTrayGroup()
    {
        m_power_rel1=m_power_rel2=0;
        m_c=false;
    }
    void serializeto(BitStream &tgt) const;
    void serializefrom(BitStream &src);
    void dump();

};
