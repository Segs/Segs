/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Powers.h"

#include <QDebug>
void PowerTrayItem::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(4,uint32_t(entry_type));
    switch(entry_type)
    {
    case TrayItemType::Power:
        tgt.StoreBits(32,powerset_idx);
        tgt.StoreBits(32,power_idx);
        break;
    case TrayItemType::Inspiration:
        tgt.StorePackedBits(3,powerset_idx);
        tgt.StorePackedBits(3,power_idx);
        break;
    case TrayItemType::Macro:
        tgt.StoreString(command);
        tgt.StoreString(short_name);
        tgt.StoreString(icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << "Unknown tray entry type" << uint32_t(entry_type);
    }
}

void PowerTrayItem::serializefrom(BitStream &src)
{
    entry_type = (TrayItemType)src.GetBits(4);
    switch(entry_type)
    {
    case TrayItemType::Power:
        powerset_idx = src.GetBits(32);
        power_idx = src.GetBits(32);
        break;
    case TrayItemType::Inspiration:
        powerset_idx = src.GetPackedBits(3);
        power_idx = src.GetPackedBits(3);
        break;
    case TrayItemType::Macro:
        src.GetString(command);
        src.GetString(short_name);
        src.GetString(icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << " Unknown tray entry type "<<uint32_t(entry_type);
    }
}

void PowerTrayItem::Dump()
{
    switch(entry_type)
    {
    case TrayItemType::Power:
    case TrayItemType::Inspiration:
            qDebug().noquote() << "[(" << QString::number(powerset_idx,16) << ',' << QString::number(power_idx,16)<<")]";
        break;
    case TrayItemType::Macro:
            qDebug() << "[(" << command << ',' << short_name<<',' << icon_name<<")]";
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << " Unknown tray entry type "<<uint32_t(entry_type);
    }

}

void PowerTrayGroup::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(32, primary_tray_idx);
    tgt.StoreBits(32, secondary_tray_idx);
    for(int bar_num=0; bar_num<9; bar_num++)
        m_trays[bar_num].serializeto(tgt);

    tgt.StoreBits(1, m_has_default_power);
    if(m_has_default_power)
    {
        tgt.StoreBits(32, m_default_powerset_idx);
        tgt.StoreBits(32, m_default_power_idx);
    }
}

void PowerTrayGroup::serializefrom(BitStream &src)
{
    primary_tray_idx = src.GetBits(32);
    secondary_tray_idx = src.GetBits(32);
    for(PowerTray &tray : m_trays)
    {
        tray.serializefrom(src);
    }
    m_has_default_power = src.GetBits(1);
    if(m_has_default_power)
    {
        m_default_powerset_idx= src.GetBits(32);
        m_default_power_idx   = src.GetBits(32);
    }
}

void PowerTrayGroup::dump()
{
    qDebug() << "primary_tray_idx: " << QString::number(primary_tray_idx,16);
    qDebug() << "secondary_tray_idx: " << QString::number(secondary_tray_idx,16);
    for(int bar_num=0; bar_num<9; bar_num++)
    {
        if(m_trays[bar_num].setPowers()==0)
            continue;
        qDebug() << "Tray: " << bar_num;
        m_trays[bar_num].Dump();
    }
    qDebug() << "m_has_default_power: " << m_has_default_power;
    if(m_has_default_power)
    {
        qDebug() << "    m_default_powerset_idx: " << QString::number(m_default_powerset_idx,16);
        qDebug() << "    m_default_power_idx: " << QString::number(m_default_power_idx,16);
    }
}

PowerTrayItem *PowerTray::getPower(size_t idx)
{
    if(idx<10)
        return &m_powers[idx];
    return nullptr;
}

int PowerTray::setPowers()
{
    int res=0;
    for(const PowerTrayItem &pow : m_powers)
    {
        res += (pow.entry_type!=TrayItemType::None);
    }
    return res;
}

void PowerTray::serializefrom(BitStream &src)
{
    for(PowerTrayItem &pow : m_powers)
        pow.serializefrom(src);
}

void PowerTray::serializeto(BitStream &tgt) const
{
    for(const PowerTrayItem &pow : m_powers)
        pow.serializeto(tgt);
}

void PowerTray::Dump()
{
    for(PowerTrayItem &pow : m_powers)
    {
        pow.Dump();
    }

}

void PowerPool_Info::serializefrom(BitStream &src)
{
    category_idx = src.GetPackedBits(3);
    powerset_idx = src.GetPackedBits(3);
    power_idx = src.GetPackedBits(3);
}

void PowerPool_Info::serializeto(BitStream &src) const
{
    src.StorePackedBits(3, category_idx);
    src.StorePackedBits(3, powerset_idx);
    src.StorePackedBits(3, power_idx);
}

//! @}
