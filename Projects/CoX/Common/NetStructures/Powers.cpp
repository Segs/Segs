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
#include "Servers/MapServer/DataHelpers.h"

#include <random>

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

/*
 * Powers Methods
 */

int getPowerCatByName(const QString &name)
{
    int idx = 0;

    const MapServerData &data(*getMapServerData());

    for(const StoredPowerCategory &pcat : data.m_all_powers.m_categories)
    {
        if(pcat.name.compare(name, Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find PowerCategory called " << name;
    return 0;
}

int getPowerSetByName(const QString &name, uint32_t pcat_idx)
{
    int idx = 0;

    const MapServerData &data(*getMapServerData());

    for(const Parse_PowerSet &pset : data.m_all_powers.m_categories[pcat_idx].m_PowerSets)
    {
        if(pset.Name.compare(name, Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find PowerSet called " << name;
    return 0;
}

int getPowerByName(const QString &name, uint32_t pcat_idx, uint32_t pset_idx)
{
    int idx = 0;

    const MapServerData &data(*getMapServerData());

    for(const Power_Data &pow : data.m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx].m_Powers)
    {
        if(pow.m_Name.compare(name, Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find Power called " << name;
    return 0;
}

CharacterPowerSet getPowers(uint32_t pcat_idx, uint32_t pset_idx)
{
    CharacterPowerSet result;
    const MapServerData &data(*getMapServerData());
    Parse_PowerSet powerset = data.m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx];

    for(const Power_Data &power : powerset.m_Powers)
    {
        int pow_idx = getPowerByName(power.m_Name, pcat_idx, pset_idx);

        CharacterPower p;
        p.m_power_tpl.m_category_idx    = pcat_idx;
        p.m_power_tpl.m_pset_idx        = pset_idx;
        p.m_power_tpl.m_pow_idx         = pow_idx;
        p.m_power_idx                   = pow_idx;
        p.m_name                        = power.m_Name;
        p.m_num_charges                 = power.m_NumCharges;
        p.m_range                       = power.Range;
        p.m_recharge_time               = power.RechargeTime;
        result.m_pset_idx               = pset_idx;

        result.m_powers.push_back(p);
    }
    return result;
}

CharacterPower *getPower(Entity &e, uint32_t pset_idx, uint32_t pow_idx)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(pset_idx > cd->m_powersets.size())
        return nullptr;

    for(int i = 0; i < cd->m_powersets.size(); i++)
    {
        for(int j = 0; j < cd->m_powersets[i].m_powers.size(); j++)
        {
            if(cd->m_powersets[i].m_powers[j].m_power_tpl.m_pset_idx == pset_idx
                    && cd->m_powersets[i].m_powers[j].m_power_tpl.m_pow_idx == pow_idx)
                return &cd->m_powersets[i].m_powers[j];
        }
    }

    qWarning() << "Failed to locate Power by index" << pset_idx << pow_idx;
    return nullptr;
}

void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t /*tgt_idx*/, uint32_t /*tgt_id*/)
{
    // Add to activepowers queue
    CharacterPower * ppower = nullptr;
    ppower = getPower(ent, pset_idx, pow_idx);

    if(ppower != nullptr && !ppower->m_name.isEmpty())
        ent.m_queued_powers.push_back(ppower);

    // TODO: Do actual power stuff. For now, be silly.
    QStringList batman_kerpow{"AIEEE!", "ARRRGH!", "AWKKKKKK!", "BAM!", "BANG!", "BAP!",
                     "BIFF!", "BLOOP!", "BLURP!", "BOFF!", "BONK!", "CLANK!",
                     "CLASH!", "CLUNK!", "CRAAACK!", "CRASH!", "CRUNCH!", "EEE-YOW!",
                     "FLRBBBBB!", "GLIPP!", "GLURPP!", "KAPOW!", "KER-PLOP!", "KLONK!",
                     "KRUNCH!", "OOOFF!", "OUCH!", "OWWW!", "PAM!", "PLOP!",
                     "POW!", "POWIE!", "QUNCKKK!", "RAKKK!", "RIP!", "SLOSH!",
                     "SOCK!", "SPLAAT!", "SWAAP!", "SWISH!", "SWOOSH!", "THUNK!",
                     "THWACK!", "THWAPP!", "TOUCHÉ!", "UGGH!", "URKK!", "VRONK!",
                     "WHACK!", "WHAMM!", "WHAP!", "ZAM!", "ZAP!", "ZGRUPPP!",
                     "ZLONK!", "ZLOPP!", "ZLOTT!", "ZOK!", "ZOWIE!", "ZWAPP!"};

    std::random_device rng;
    std::mt19937 urng(rng());
    std::shuffle(batman_kerpow.begin(), batman_kerpow.end(), urng);
    QString contents = batman_kerpow.first();

    sendFloatingInfo(&ent, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}


/*
 * Inspirations Methods
 */
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
                // TODO: This currently crashes client upon next login
                //insp.m_col = dest_col;
                //insp.m_row = dest_row;
            }
        }
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to move inspiration from" << src_col << src_row << "to" << dest_col << dest_row;
}

void useInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    removeInspiration(ent, col, row);
    // TODO: Do inspiration benefit

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to use inspiration from" << col << row;
}

void removeInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    auto iter = std::find_if( ent.m_char->m_char_data.m_inspirations.begin(),
                              ent.m_char->m_char_data.m_inspirations.end(),
                              [col, row](const CharacterInspiration& insp)->bool {return col==insp.m_col && row==insp.m_row;});
    if(iter != ent.m_char->m_char_data.m_inspirations.end())
    {
        // This currently crashes the cilent upon next login
        //iter = ent.m_char->m_char_data.m_inspirations.erase(iter);
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to remove inspiration from" << col << row;
}

//! @}
