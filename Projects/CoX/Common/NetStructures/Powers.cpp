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
    tgt.StoreBits(4, uint32_t(m_entry_type));
    switch(m_entry_type)
    {
    case TrayItemType::Power:
        tgt.StoreBits(32, m_pset_idx);
        tgt.StoreBits(32, m_pow_idx);
        break;
    case TrayItemType::Inspiration:
        tgt.StorePackedBits(3, m_pset_idx);
        tgt.StorePackedBits(3, m_pow_idx);
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

    for(int bar_num = 0; bar_num < 9; bar_num++)
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
    for(int i = 0; i < 10; i++)
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

CharacterPower getPowerData(uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx)
{
    const MapServerData &data(*getMapServerData());
    Power_Data power = data.m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx].m_Powers[pow_idx];

    CharacterPower result;
    result.m_power_tpl.m_category_idx    = pcat_idx;
    result.m_power_tpl.m_pset_idx        = pset_idx;
    result.m_power_tpl.m_pow_idx         = pow_idx;
    result.m_name                        = power.m_Name;
    result.m_index                       = pow_idx;
    result.m_num_charges                 = power.m_NumCharges;
    result.m_range                       = power.Range;
    result.m_recharge_time               = power.RechargeTime;

    //if(logPowers().isDebugEnabled())
        //dumpPower(result);

    return result;
}

CharacterPowerSet getPowerSetData(uint32_t pcat_idx, uint32_t pset_idx)
{
    CharacterPowerSet result;
    const MapServerData &data(*getMapServerData());
    Parse_PowerSet powerset = data.m_all_powers.m_categories[pcat_idx].m_PowerSets[pset_idx];

    for(int pow_idx = 0; pow_idx < powerset.m_Powers.size(); pow_idx++)
    {
        CharacterPower p = getPowerData(pcat_idx, pset_idx, pow_idx);
        result.m_powers.push_back(p);
    }
    return result;
}

CharacterPower *getOwnedPower(Entity &e, uint32_t pset_idx, uint32_t pow_idx)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(pset_idx > cd->m_powersets.size())
        return nullptr;

    for(int i = 0; i < cd->m_powersets.size(); i++)
    {
        for(int j = 0; j < cd->m_powersets[i].m_powers.size(); j++)
        {
            if(cd->m_powersets[i].m_index == pset_idx
                    && cd->m_powersets[i].m_powers[j].m_index == pow_idx)
            {
                qCDebug(logPowers) << "getPower returned" << cd->m_powersets[i].m_powers.at(j).m_name;
                return &cd->m_powersets[i].m_powers[j];
            }
        }
    }

    qWarning() << "Failed to locate Power by index" << pset_idx << pow_idx;
    return nullptr;
}

void addPower(CharacterData &cd, uint32_t pcat_idx, uint32_t pset_idx, uint32_t pow_idx)
{
    CharacterPowerSet pset;
    CharacterPower power;

    power = getPowerData(pcat_idx, pset_idx, pow_idx);
    pset.m_level_bought = cd.m_level;
    power.m_index = pow_idx;
    pset.m_index = pset_idx;
    pset.m_category = pcat_idx;

    // Does Power or PowerSet already exist?
    int existing_pset = 0;
    for(CharacterPowerSet &ps : cd.m_powersets)
    {
        if(ps.m_category == pcat_idx && ps.m_index == pset_idx)
        {
            // Does Power already exist? Then return.
            for(CharacterPower &p : ps.m_powers)
            {
                if(p.m_index == pow_idx)
                {
                    qCDebug(logPowers) << "Power already exists " << ps.m_category << pset_idx << pow_idx;
                    return;
                }
            }

            qCDebug(logPowers) << "Character adding power to existing powerset " << ps.m_category << pset_idx << pow_idx;
            cd.m_powersets[existing_pset].m_powers.push_back(power); // if powerset already exists redefine pset

            if(logPowers().isDebugEnabled())
                dumpPower(power);

            return;
        }

        existing_pset++;
    }

    // Add power to vector
    pset.m_powers.push_back(power);

    qCDebug(logPowers) << "Adding Power:" << pcat_idx << pset_idx << pow_idx;
    if(logPowers().isDebugEnabled())
        dumpPower(power);

    // Add powerset to vector
    cd.m_powersets.push_back(pset);
}

void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t /*tgt_idx*/, uint32_t /*tgt_id*/)
{
    // Add to activepowers queue
    CharacterPower * ppower = nullptr;
    ppower = getOwnedPower(ent, pset_idx, pow_idx);

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

void dumpPowerPoolInfo(const PowerPool_Info &pinfo)
{
    qDebug().nospace().noquote() << QString("  PPInfo: %1 %2 %3")
                                    .arg(pinfo.m_category_idx)
                                    .arg(pinfo.m_pset_idx)
                                    .arg(pinfo.m_pow_idx);
}

void dumpPower(const CharacterPower &pow)
{
    qDebug().noquote() << pow.m_name;
    qDebug().noquote() << "  Index: " << pow.m_index;
    dumpPowerPoolInfo(pow.m_power_tpl);
    qDebug().noquote() << "  LevelBought: " << pow.m_level_bought;
    qDebug().noquote() << "  NumCharges: " << pow.m_num_charges;
    qDebug().noquote() << "  UsageTime: " << pow.m_usage_time;
    qDebug().noquote() << "  ActivationTime: " << pow.m_activation_time;
    qDebug().noquote() << "  Range: " << pow.m_range;
    qDebug().noquote() << "  RechargeTime: " << pow.m_recharge_time;
    qDebug().noquote() << "  ActivationState: " << pow.m_activation_state;
    qDebug().noquote() << "  ActivationStateChange: " << pow.m_active_state_change;
    qDebug().noquote() << "  TimerUpdated: " << pow.m_timer_updated;
    qDebug().noquote() << "  NumEnhancements: " << pow.m_num_enhancements;
}

void dumpOwnedPowers(CharacterData &cd)
{
    int pset_idx = 0;

    for(CharacterPowerSet &pset : cd.m_powersets)
    {
        int pow_idx = 0;
        qDebug().noquote() << "Category: " << pset.m_category;
        qDebug().noquote() << "PowerSet: " << pset.m_index << "Iterator:" << pset_idx;
        qDebug().noquote() << "LevelBought: " << pset.m_level_bought;

        for(CharacterPower &p : pset.m_powers)
        {
            qDebug().noquote() << "Power: " << p.m_index << "Iterator:" << pow_idx;
            dumpPower(p);
            pow_idx++;
        }

        pset_idx++;
    }
}


/*
 * Inspirations Methods
 */
void addInspirationByName(CharacterData &cd, QString &name)
{
    const MapServerData &data(*getMapServerData());
    uint32_t pcat_idx = getPowerCatByName("Inspirations");
    uint32_t pset_idx, pow_idx    = 0;
    bool found  = false;

    int i = 0;
    for(const Parse_PowerSet &pset : data.m_all_powers.m_categories[pcat_idx].m_PowerSets)
    {
        int j = 0;
        for(const Power_Data &pow : pset.m_Powers)
        {
            if(pow.m_Name.compare(name, Qt::CaseInsensitive) == 0)
            {
                qCDebug(logPowers) << pow.m_Name << i << j;
                found = true;
                pset_idx = i;
                pow_idx = j;
            }

            j++;
        }

        i++;
    }

    if(!found)
    {
        qCDebug(logPowers) << "Failed to find Inspiration called " << name;
        return;
    }

    CharacterInspiration insp;
    insp.m_has_insp = true;
    insp.m_name = name;
    insp.m_insp_tpl.m_category_idx = pcat_idx;
    insp.m_insp_tpl.m_pset_idx = pset_idx;
    insp.m_insp_tpl.m_pow_idx = pow_idx;

    addInspirationToChar(cd, insp);
}

void addInspirationToChar(CharacterData &cd, CharacterInspiration insp)
{
    int max_cols = cd.m_max_insp_cols;
    int max_rows = cd.m_max_insp_rows;

    if(cd.m_inspirations.size() > max_cols*max_rows)
    {
        qCDebug(logPowers) << "Character cannot hold any more inspirations";
        return;
    }

    cd.m_inspirations.push_back(insp);
    qCDebug(logPowers) << "Character received inspiration:"
                       << insp.m_insp_tpl.m_category_idx
                       << insp.m_insp_tpl.m_pset_idx
                       << insp.m_insp_tpl.m_pow_idx;
}

int getInspirationIdx(Entity &ent, uint32_t col, uint32_t row)
{
    vInspirations *insp_arr = &ent.m_char->m_char_data.m_inspirations;
    int max_cols = ent.m_char->m_char_data.m_max_insp_cols;
    int max_rows = ent.m_char->m_char_data.m_max_insp_rows;

    if(col > max_cols || row > max_rows)
    {
        qWarning() << "Inspiration doesn't exist at" << col << row;
        return -1;
    }

    auto iter = std::find_if(insp_arr->begin(), insp_arr->end(),
                              [col, row](const CharacterInspiration& insp)->bool {return col==insp.m_col && row==insp.m_row;});

    if(iter != insp_arr->end())
    {
        qCDebug(logPowers) << "Entity: " << ent.m_idx << " returning inspiration at " << col << row;
        return std::distance(insp_arr->begin(), iter);
    }

    qCDebug(logPowers) << "Entity: " << ent.m_idx << " does not have an inspiration at " << col << "x" << row;
    return -1;
}

void moveInspiration(Entity &ent, uint32_t src_col, uint32_t src_row, uint32_t dest_col, uint32_t dest_row)
{
    vInspirations *insp_arr = &ent.m_char->m_char_data.m_inspirations;
    CharacterInspiration src_insp, dest_insp;
    int max_cols = ent.m_char->m_char_data.m_max_insp_cols;
    int max_rows = ent.m_char->m_char_data.m_max_insp_rows;
    bool dest_is_empty = false;

    if(dest_col > max_cols || dest_row > max_rows)
    {
        removeInspiration(ent, src_col, src_row);
        return;
    }

    int src_idx = getInspirationIdx(ent, src_col, src_row);
    if(src_idx < 0)
        return;

    // src is now dest (even if it was empty)
    insp_arr->at(src_idx).m_col = dest_col;
    insp_arr->at(src_idx).m_row = dest_row;

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to move inspiration from" << src_col << src_row << "to" << dest_col << dest_row;

    int dest_idx = getInspirationIdx(ent, dest_col, dest_row);
    if(dest_idx < 0)
        return;

    // dest is now src
    insp_arr->at(dest_idx).m_col = src_col;
    insp_arr->at(dest_idx).m_row = src_row;
}

void useInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    int insp_idx = getInspirationIdx(ent, col, row);
    if(insp_idx < 0)
        return;

    removeInspiration(ent, col, row);
    // TODO: Do inspiration benefit

    qCDebug(logPowers) << "Entity: " << ent.m_idx << "wants to use inspiration from" << col << row;
    QString contents = "Inspired!";
    sendFloatingInfo(&ent, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}

void removeInspiration(Entity &ent, uint32_t col, uint32_t row)
{
    int insp_idx = getInspirationIdx(ent, col, row);
    if(insp_idx < 0)
        return;

    qCDebug(logPowers) << "Entity: " << ent.m_idx << " wants to remove inspiration from " << col << "x" << row;
    ent.m_char->m_char_data.m_inspirations.erase(ent.m_char->m_char_data.m_inspirations.begin() + insp_idx);
}

void dumpInspirations(CharacterData &cd)
{
    if(cd.m_inspirations.size() == 0)
        qDebug().noquote() << "This character has no inspirations.";

    for(CharacterInspiration &insp : cd.m_inspirations)
    {
        qDebug().noquote() << "Inspiration: " << insp.m_name;
        qDebug().noquote() << "  HasInsp: " << insp.m_has_insp;
        qDebug().noquote() << "  Col: " << insp.m_col;
        qDebug().noquote() << "  Row: " << insp.m_row;
        qDebug().noquote() << "  CategoryIdx: " << insp.m_insp_tpl.m_category_idx;
        qDebug().noquote() << "  PowerSetIdx: " << insp.m_insp_tpl.m_pset_idx;
        qDebug().noquote() << "  PowerIdx: " << insp.m_insp_tpl.m_pow_idx;
    }
}

//! @}
