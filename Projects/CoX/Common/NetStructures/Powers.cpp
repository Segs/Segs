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

#include "Entity.h"
#include "Character.h"
#include "Powers.h"
#include "Logging.h"
#include "Servers/MapServer/MapServerData.h"

void PowerTrayItem::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(4,uint32_t(m_entry_type));
    switch(m_entry_type)
    {
    case TrayItemType::Power:
        tgt.StoreBits(32,m_pset_idx);
        tgt.StoreBits(32,m_pow_idx);
        break;
    case TrayItemType::Inspiration:
        tgt.StorePackedBits(3,m_pset_idx);
        tgt.StorePackedBits(3,m_pow_idx);
        break;
    case TrayItemType::Macro:
        tgt.StoreString(m_command);
        tgt.StoreString(m_short_name);
        tgt.StoreString(m_icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << "Unknown tray entry type" << uint32_t(m_entry_type);
    }
}

void PowerTrayItem::serializefrom(BitStream &src)
{
    m_entry_type = (TrayItemType)src.GetBits(4);
    switch(m_entry_type)
    {
    case TrayItemType::Power:
        m_pset_idx = src.GetBits(32);
        m_pow_idx = src.GetBits(32);
        break;
    case TrayItemType::Inspiration:
        m_pset_idx = src.GetPackedBits(3);
        m_pow_idx = src.GetPackedBits(3);
        break;
    case TrayItemType::Macro:
        src.GetString(m_command);
        src.GetString(m_short_name);
        src.GetString(m_icon_name);
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << " Unknown tray entry type " << uint32_t(m_entry_type);
    }
}

void PowerTrayItem::Dump()
{
    switch(m_entry_type)
    {
    case TrayItemType::Power:
    case TrayItemType::Inspiration:
            qDebug().noquote() << "[(" << QString::number(m_pset_idx,16) << ',' << QString::number(m_pow_idx,16)<<")]";
        break;
    case TrayItemType::Macro:
            qDebug() << "[(" << m_command << ',' << m_short_name<<',' << m_icon_name<<")]";
        break;
    case TrayItemType::None:
        break;
    default:
        qWarning() << " Unknown tray entry type "<<uint32_t(m_entry_type);
    }

}

void PowerTrayGroup::serializeto(BitStream &tgt) const
{
    tgt.StoreBits(32, m_primary_tray_idx);
    tgt.StoreBits(32, m_second_tray_idx);

    for(int bar_num=0; bar_num<9; bar_num++)
        m_trays[bar_num].serializeto(tgt);

    tgt.StoreBits(1, m_has_default_power);
    if(m_has_default_power)
    {
        tgt.StoreBits(32, m_default_pset_idx);
        tgt.StoreBits(32, m_default_pow_idx);
    }
}

void PowerTrayGroup::serializefrom(BitStream &src)
{
    m_primary_tray_idx = src.GetBits(32);
    m_second_tray_idx = src.GetBits(32);
    for(PowerTray &tray : m_trays)
    {
        tray.serializefrom(src);
    }
    m_has_default_power = src.GetBits(1);
    if(m_has_default_power)
    {
        m_default_pset_idx= src.GetBits(32);
        m_default_pow_idx   = src.GetBits(32);
    }
}

void PowerTrayGroup::dump()
{
    qDebug() << "primary_tray_idx: " << QString::number(m_primary_tray_idx,16);
    qDebug() << "secondary_tray_idx: " << QString::number(m_second_tray_idx,16);
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
        qDebug() << "    m_default_powerset_idx: " << QString::number(m_default_pset_idx,16);
        qDebug() << "    m_default_power_idx: " << QString::number(m_default_pow_idx,16);
    }
}

PowerTrayItem *PowerTray::getPowerTrayItem(size_t idx)
{
    if(idx<10)
        return &m_tray_items[idx];
    return nullptr;
}

int PowerTray::setPowers()
{
    int res=0;
    for(const PowerTrayItem &pow : m_tray_items)
    {
        res += (pow.m_entry_type!=TrayItemType::None);
    }
    return res;
}

void PowerTray::serializefrom(BitStream &src)
{
    for(PowerTrayItem &pow : m_tray_items)
        pow.serializefrom(src);
}

void PowerTray::serializeto(BitStream &tgt) const
{
    for(int i=0; i<10; i++)
        m_tray_items[i].serializeto(tgt);
}

void PowerTray::Dump()
{
    for(PowerTrayItem &pow : m_tray_items)
    {
        pow.Dump();
    }

}

void PowerPool_Info::serializefrom(BitStream &src)
{
    m_category_idx = src.GetPackedBits(3);
    m_pset_idx = src.GetPackedBits(3);
    m_pow_idx = src.GetPackedBits(3);
}

void PowerPool_Info::serializeto(BitStream &src) const
{
    src.StorePackedBits(3, m_category_idx);
    src.StorePackedBits(3, m_pset_idx);
    src.StorePackedBits(3, m_pow_idx);
}

void moveInspiration(Entity &ent, uint32_t src_col, uint32_t src_row, uint32_t dest_col, uint32_t dest_row)
{
    for(CharacterInspiration &insp : ent.m_char->m_char_data.m_inspirations)
    {
        if(src_col == insp.m_col && src_row == insp.m_row)
        {
            if(dest_col > ent.m_char->m_char_data.m_max_insp_cols
                    || dest_row > ent.m_char->m_char_data.m_max_insp_rows)
            {
                removeInspiration(ent, src_col, src_row);
            }
            else
            {
                insp.m_col = dest_col;
                insp.m_row = dest_row;
            }
        }
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to move inspiration from" << src_col << src_row << "to" << dest_col << dest_row;
}

void useInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    auto it = std::begin(ent.m_char->m_char_data.m_inspirations);
    for(CharacterInspiration &insp : ent.m_char->m_char_data.m_inspirations)
    {
        if(col == insp.m_col && row == insp.m_row)
        {
            // TODO: Do inspiration benefit
            ent.m_char->m_char_data.m_inspirations.erase(it);
        }

        ++it;
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to use inspiration from" << col << row;
}

void removeInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    auto it = std::begin(ent.m_char->m_char_data.m_inspirations);
    for(CharacterInspiration &insp : ent.m_char->m_char_data.m_inspirations)
    {
        if(col == insp.m_col && row == insp.m_row)
        {
            ent.m_char->m_char_data.m_inspirations.erase(it);
        }

        ++it;
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to remove inspiration from" << col << row;
}

//! @}
