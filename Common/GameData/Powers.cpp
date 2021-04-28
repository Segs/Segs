/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Powers.h"
#include "Entity.h"
#include "EntityHelpers.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Components/Logging.h"
#include "GameData/GameDataStore.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

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
        res += (pow.m_entry_type!=TrayItemType::None);

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
        pow.Dump();
}

void PowerPool_Info::serializefrom(BitStream &src)
{
    m_pcat_idx = src.GetPackedBits(3);
    m_pset_idx = src.GetPackedBits(3);
    m_pow_idx = src.GetPackedBits(3);
}

void PowerPool_Info::serializeto(BitStream &src) const
{
    src.StorePackedBits(3, m_pcat_idx);
    src.StorePackedBits(3, m_pset_idx);
    src.StorePackedBits(3, m_pow_idx);
}

template<class Archive>
void PowerPool_Info::serialize(Archive &archive, uint32_t const version)
{
    if(version != PowerPool_Info::class_version)
    {
        qCritical() << "Failed to serialize PowerPool_Info, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("CategoryIdx", m_pcat_idx));
    archive(cereal::make_nvp("PowersetIdx", m_pset_idx));
    archive(cereal::make_nvp("PowerIdx", m_pow_idx));
}
CEREAL_CLASS_VERSION(PowerPool_Info, PowerPool_Info::class_version)   // register PowerPool_Info class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PowerPool_Info)
/*
 * Powers Methods
 */

int getPowerCatByName(const QString &name)
{
    int idx = 0;

    for(const StoredPowerCategory &pcat : getGameData().m_all_powers.m_categories)
    {
        if(name.compare(pcat.name, Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find PowerCategory called " << name;
    return 0;
}

int getPowerSetByName(const QString &name, uint32_t pcat_idx)
{
    int idx = 0;

    for(const Parse_PowerSet &pset : getGameData().get_power_category(pcat_idx).m_PowerSets)
    {
        if(name.compare(pset.m_Name,Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find PowerSet called " << name;
    return 0;
}

int getPowerByName(const QString &name, uint32_t pcat_idx, uint32_t pset_idx)
{
    int idx = 0;

    for(const Power_Data &pow : getGameData().get_powerset(pcat_idx, pset_idx).m_Powers)
    {
        if(name.compare(pow.m_Name, Qt::CaseInsensitive) == 0)
            return idx;

        idx++;
    }

    qCDebug(logPowers) << "Failed to find Power called " << name;
    return 0;
}

CharacterPower getPowerData(PowerPool_Info &ppool)
{
    Power_Data power = getGameData().get_power_template(ppool.m_pcat_idx, ppool.m_pset_idx, ppool.m_pow_idx);

    CharacterPower result;
    result.m_power_info.m_pcat_idx  = ppool.m_pcat_idx;
    result.m_power_info.m_pset_idx  = ppool.m_pset_idx;
    result.m_power_info.m_pow_idx   = ppool.m_pow_idx;
    result.m_index                  = ppool.m_pow_idx;
    result.m_charges_remaining      = power.m_NumCharges;
    result.m_activate_period        = power.ActivatePeriod;
    result.m_usage_time             = power.m_UsageTime;

    if(power.m_NumCharges > 0)
        result.m_is_limited = true;

    //if(logPowers().isDebugEnabled())
        //dumpPower(result);

    return result;
}

CharacterPowerSet getPowerSetData(PowerPool_Info &ppool)
{
    CharacterPowerSet result;
    Parse_PowerSet powerset = getGameData().get_powerset(ppool.m_pcat_idx, ppool.m_pset_idx);

    for(uint32_t pow_idx = 0; pow_idx < powerset.m_Powers.size(); ++pow_idx)
    {
        ppool.m_pow_idx = pow_idx;
        CharacterPower p = getPowerData(ppool);
        result.m_powers.push_back(p);
    }
    return result;
}

PowerVecIndexes getOwnedPowerIndexes(Entity &e, const PowerPool_Info &ppool)
{
    PowerVecIndexes vec_indexes;

    int pset_idx_in_vec = 0;
    for(CharacterPowerSet &pset : e.m_char->m_char_data.m_powersets)
    {
        int pow_idx_in_vec = 0;
        for(CharacterPower &pow : pset.m_powers)
        {
            if(pow.m_power_info.m_pcat_idx == ppool.m_pcat_idx
                    && pow.m_power_info.m_pset_idx == ppool.m_pset_idx
                    && pow.m_power_info.m_pow_idx == ppool.m_pow_idx)
            {
                vec_indexes.m_pset_vec_idx = pset_idx_in_vec;
                vec_indexes.m_pow_vec_idx = pow_idx_in_vec;
                return vec_indexes;
            }
            ++pow_idx_in_vec;
        }
        ++pset_idx_in_vec;
    }

    return vec_indexes;
}

CharacterPower *getOwnedPowerByVecIdx(Entity &e, uint32_t pset_idx, uint32_t pow_idx)
{
    bool fail = false;
    CharacterData *cd = &e.m_char->m_char_data;

    if(pset_idx >= cd->m_powersets.size())
        fail = true;
    else if(pow_idx > cd->m_powersets.at(pset_idx).m_powers.size()) // must check after confirming pset_idx is valid
        fail = true;

    if(fail)
    {
        qWarning() << "Failed to locate Power by index."
                   << pset_idx
                   << pow_idx;
        return nullptr;
    }

    qCDebug(logPowers) << "getPower returned" << cd->m_powersets[pset_idx].m_powers[pow_idx].getPowerTemplate().m_Name;

    return &cd->m_powersets.at(pset_idx).m_powers.at(pow_idx);
}

CharacterPower *getOwnedPowerByTpl(Entity &e, const PowerPool_Info &ppool)
{
    PowerVecIndexes vec_indexes = getOwnedPowerIndexes(e, ppool);

    return getOwnedPowerByVecIdx(e, vec_indexes.m_pset_vec_idx, vec_indexes.m_pow_vec_idx);
}

void addPowerSet(CharacterData &cd, PowerPool_Info &ppool)
{
    CharacterPowerSet pset;

    qCDebug(logPowers) << "Adding empty PowerSet:" << ppool.m_pcat_idx << ppool.m_pset_idx;

    pset.m_level_bought = cd.m_level;
    pset.m_index        = ppool.m_pset_idx;
    pset.m_category     = ppool.m_pcat_idx;

    cd.m_powersets.push_back(pset);
    cd.m_has_updated_powers = true; // update client on power status
    cd.m_reset_powersets = true; // possible that we need to reset the powerset array client side
}

void addEntirePowerSet(CharacterData &cd, PowerPool_Info &ppool)
{
    CharacterPowerSet pset;

    qCDebug(logPowers) << "Adding entire PowerSet:" << ppool.m_pcat_idx << ppool.m_pset_idx;

    pset = getPowerSetData(ppool);
    pset.m_level_bought = cd.m_level;
    pset.m_index        = ppool.m_pset_idx;
    pset.m_category     = ppool.m_pcat_idx;

    cd.m_powersets.push_back(pset);
    cd.m_has_updated_powers = true; // update client on power status
    cd.m_reset_powersets = true; // possible that we need to reset the powerset array client side
}

void addPower(CharacterData &cd, PowerPool_Info &ppool)
{
    CharacterPowerSet new_pset;
    new_pset.m_category     = ppool.m_pcat_idx;
    new_pset.m_index        = ppool.m_pset_idx;
    new_pset.m_level_bought = cd.m_level;

    CharacterPower new_power;
    new_power               = getPowerData(ppool);
    new_power.m_index       = ppool.m_pow_idx;
    new_power.m_level_bought = cd.m_level;

    // powers start with 1 free eh slot
    reserveEnhancementSlot(&new_power, cd.m_level);

    // Does Power or PowerSet already exist?
    int existing_pset = 0;
    for(CharacterPowerSet &ps : cd.m_powersets)
    {
        if(ps.m_category == new_pset.m_category
                && ps.m_index == new_pset.m_index)
        {
            // Does Power already exist? Then return.
            for(CharacterPower &p : ps.m_powers)
            {
                if(p.m_index == new_power.m_index)
                {
                    qCDebug(logPowers) << "Power already exists " << ps.m_category << new_pset.m_index << new_power.m_index;
                    return;
                }
            }

            qCDebug(logPowers) << "Character adding power to existing powerset " << ps.m_category << new_pset.m_index << new_power.m_index;
            cd.m_powersets[existing_pset].m_powers.push_back(new_power); // if powerset already exists redefine pset

            if(logPowers().isDebugEnabled())
                dumpPower(new_power);

            cd.m_has_updated_powers = true; // update client on power status
            return;
        }

        existing_pset++;
    }

    // Add power to vector
    new_pset.m_powers.push_back(new_power);

    qCDebug(logPowers) << "Adding Power:" << new_pset.m_category << new_pset.m_index << new_power.m_index;
    if(logPowers().isDebugEnabled())
        dumpPower(new_power);

    // Add powerset to vector
    cd.m_powersets.push_back(new_pset);
    cd.m_has_updated_powers = true; // update client on power status
    cd.m_reset_powersets = true; // possible that we need to reset the powerset array client side
}

void removePower(CharacterData &cd, const PowerPool_Info &ppool)
{
    for(CharacterPowerSet &pset : cd.m_powersets)
    {
        auto iter = std::find_if(pset.m_powers.begin(), pset.m_powers.end(),
                                  [ppool](const CharacterPower& pow)->bool {return ppool.m_pow_idx==pow.m_index;});

        if(iter != pset.m_powers.end())
        {
            qCDebug(logPowers) << "Removing Power:" << ppool.m_pcat_idx << ppool.m_pset_idx << ppool.m_pow_idx;
            pset.m_powers.erase(iter);
            cd.m_has_updated_powers = true; // update client on power status
            cd.m_reset_powersets = true; // possible that we need to reset the powerset array client side

            return;
        }
    }

    qCDebug(logPowers) << "Player does not own Power:" << ppool.m_pcat_idx << ppool.m_pset_idx << ppool.m_pow_idx;
}

uint32_t countAllOwnedPowers(CharacterData &cd, bool include_temps)
{
    uint32_t count = 0;

    for(const CharacterPowerSet &pset : cd.m_powersets)
    {
        if(pset.m_index == 0 && !include_temps) // don't count temporary_powers
            continue;

        count += pset.m_powers.size(); // total up all powers
    }

    qCDebug(logPowers) << "Total Owned Powers:" << include_temps << count;
    return count;
}

void dumpPowerPoolInfo(const PowerPool_Info &pinfo)
{
    qDebug().nospace().noquote() << QString("  PPInfo: %1 %2 %3")
                                    .arg(pinfo.m_pcat_idx)
                                    .arg(pinfo.m_pset_idx)
                                    .arg(pinfo.m_pow_idx);
}

void dumpPower(const CharacterPower &pow)
{
    Power_Data power_tpl(pow.getPowerTemplate());
    qDebug().noquote() << power_tpl.m_Name;
    qDebug().noquote() << "  Index: " << pow.m_index;
    dumpPowerPoolInfo(pow.m_power_info);
    qDebug().noquote() << "  LevelBought: " << pow.m_level_bought;
    qDebug().noquote() << "  NumCharges: " << power_tpl.m_NumCharges;
    qDebug().noquote() << "  UsageTime: " << power_tpl.m_UsageTime;
    qDebug().noquote() << "  ActivationTime: " << power_tpl.ActivatePeriod;
    qDebug().noquote() << "  Range: " << power_tpl.Range;
    qDebug().noquote() << "  RechargeTime: " << power_tpl.RechargeTime;
    qDebug().noquote() << "  ActivationState: " << pow.m_activation_state;
    qDebug().noquote() << "  ActivationStateChange: " << pow.m_active_state_change;
    qDebug().noquote() << "  TimerUpdated: " << pow.m_timer_updated;
    qDebug().noquote() << "  NumEnhancements: " << pow.m_total_eh_slots;
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
    CharacterInspiration insp;
    uint32_t pcat_idx = getPowerCatByName("Inspirations");
    uint32_t pset_idx, pow_idx    = 0;
    bool found  = false;

    int i = 0;
    for(const Parse_PowerSet &pset : getGameData().get_power_category(pcat_idx).m_PowerSets)
    {
        int j = 0;
        for(const Power_Data &pow : pset.m_Powers)
        {
            if(name.compare(pow.m_Name, Qt::CaseInsensitive) == 0)
            {
                qCDebug(logPowers) << pow.m_Name << i << j;
                found = true;
                pset_idx = i;
                pow_idx = j;
                insp.m_insp_tpl = pow;
                break;
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

    insp.m_has_insp = true;
    insp.m_name = name;
    insp.m_insp_info.m_pcat_idx = pcat_idx;
    insp.m_insp_info.m_pset_idx = pset_idx;
    insp.m_insp_info.m_pow_idx = pow_idx;

    addInspirationToChar(cd, insp);
}

void addInspirationToChar(CharacterData &cd, const CharacterInspiration& insp)
{
    const uint32_t max_cols = cd.m_max_insp_cols;
    const uint32_t max_rows = cd.m_max_insp_rows;

    for (uint32_t row = 0; row < max_rows; ++row)
    {
        for (uint32_t col = 0; col < max_cols; ++col)
        {
            if(!cd.m_inspirations.at(col, row).m_has_insp)
            {
                cd.m_inspirations.at(col, row) = insp;
                cd.m_inspirations.at(col, row).m_col = col;
                cd.m_inspirations.at(col, row).m_row = row;
                cd.m_inspirations.at(col, row).m_has_insp = true;
                cd.m_has_updated_powers = true; // update client on power status

                qCDebug(logPowers) << "Character received inspiration:"
                                   << insp.m_insp_info.m_pcat_idx
                                   << insp.m_insp_info.m_pset_idx
                                   << insp.m_insp_info.m_pow_idx;
                return;
            }
        }
    }

    qCDebug(logPowers) << "Character cannot hold any more inspirations";
}

const CharacterInspiration* getInspiration(const Entity &ent, uint32_t col, uint32_t row)
{
    const CharacterData& cd = ent.m_char->m_char_data;
    const uint32_t max_cols = cd.m_max_insp_cols;
    const uint32_t max_rows = cd.m_max_insp_rows;

    if(col >= max_cols)
    {
        qCWarning(logPowers) << "getInspiration: Invalid inspiration column:" << col;
        return nullptr;
    }

    if(row >= max_rows)
    {
        qCWarning(logPowers) << "getInspiration: Invalid inspiration row:" << row;
        return nullptr;
    }

    const CharacterInspiration& insp = cd.m_inspirations.at(col, row);
    if(!insp.m_has_insp)
    {
        qCWarning(logPowers) << "getInspiration: No inspiration at col:" << col << "row:" << row;
        return nullptr;
    }

    return &insp;
}

int getNumberInspirations(const CharacterData &cd)
{
    const uint32_t max_cols = cd.m_max_insp_cols;
    const uint32_t max_rows = cd.m_max_insp_rows;
    int count = 0;

    for(uint32_t row = 0; row < max_rows; ++row)
    {
        for(uint32_t col = 0; col < max_cols; ++col)
        {
            if(cd.m_inspirations.at(col, row).m_has_insp)
                ++count;
        }
    }

    return count;
}

int getMaxNumberInspirations(const CharacterData &cd)
{
    return static_cast<int>(cd.m_max_insp_cols * cd.m_max_insp_rows);
}

void moveInspiration(CharacterData &cd, uint32_t src_col, uint32_t src_row, uint32_t dest_col, uint32_t dest_row)
{
    vInspirations *insp_arr = &cd.m_inspirations;
    uint32_t max_cols = cd.m_max_insp_cols;
    uint32_t max_rows = cd.m_max_insp_rows;

    if(dest_col > max_cols || dest_row > max_rows)
    {
        removeInspiration(cd, src_col, src_row);
        return;
    }

    // save src_insp incase of switch
    insp_arr->at(src_col, src_row).m_col = dest_col;
    insp_arr->at(src_col, src_row).m_row = dest_row;
    insp_arr->at(dest_col, dest_row).m_col = src_col;
    insp_arr->at(dest_col, dest_row).m_row = src_row;
    std::swap(insp_arr->at(src_col, src_row), insp_arr->at(dest_col, dest_row));

    cd.m_has_updated_powers = true; // update client on power status

    qCDebug(logPowers) << "Moving inspiration from" << src_col << "x" << src_row << "to" << dest_col << "x" << dest_row;
}

void removeInspiration(CharacterData &cd, uint32_t col, uint32_t row)
{
    qCDebug(logPowers) << "Removing inspiration from " << col << "x" << row;

    int max_rows = cd.m_max_insp_rows;
    CharacterInspiration insp;
    insp.m_col = col;
    insp.m_row = row;
    cd.m_inspirations.at(col, row) = insp;
    for(int j = row; j < max_rows; ++j)
    {
        if(j+1 >= max_rows)
            break;

        moveInspiration(cd, col, j+1, col, j);
    }

    cd.m_has_updated_powers = true; // update client on power status
}


void dumpInspirations(CharacterData &cd)
{
    int max_cols = cd.m_max_insp_cols;
    int max_rows = cd.m_max_insp_rows;

    if(cd.m_inspirations.size() == 0)
        qDebug().noquote() << "This character has no inspirations.";

    for(int i = 0; i < max_cols; ++i)
    {
        for(int j = 0; j < max_rows; ++j)
        {
            qDebug().noquote() << "Inspiration: " << cd.m_inspirations.at(i, j).m_name;
            qDebug().noquote() << "  HasInsp: " << cd.m_inspirations.at(i, j).m_has_insp;
            qDebug().noquote() << "  Col: " << cd.m_inspirations.at(i, j).m_col;
            qDebug().noquote() << "  Row: " << cd.m_inspirations.at(i, j).m_row;
            qDebug().noquote() << "  CategoryIdx: " << cd.m_inspirations.at(i, j).m_insp_info.m_pcat_idx;
            qDebug().noquote() << "  PowerSetIdx: " << cd.m_inspirations.at(i, j).m_insp_info.m_pset_idx;
            qDebug().noquote() << "  PowerIdx: " << cd.m_inspirations.at(i, j).m_insp_info.m_pow_idx;
        }
    }
}

/*
 * Enhancements (boosts) Methods
 */
void addEnhancementByName(CharacterData &cd, QString &name, uint32_t &level)
{
    CharacterEnhancement enhance;
    uint32_t pcat_idx = getPowerCatByName("Boosts");
    uint32_t pset_idx, pow_idx    = 0;
    bool found  = false;

    if(getNumberEnhancements(cd) == cd.m_enhancements.size())
    {
        qCDebug(logPowers) << "Enhancement tray is full!";
        return;
    }

    int i = 0;
    for(const Parse_PowerSet &pset : getGameData().get_power_category(pcat_idx).m_PowerSets)
    {
        if(name.compare(pset.m_Name, Qt::CaseInsensitive) == 0)
        {
            qCDebug(logPowers) << pset.m_Name << i;
            found = true;
            pset_idx = i;
            enhance.m_enhance_tpl = pset.m_Powers.front();
            break;
        }

        i++;
    }

    if(!found)
    {
        qCDebug(logPowers) << "Failed to find Enhancement called " << name;
        return;
    }

    enhance.m_slot_used                     = true;
    enhance.m_slot_idx                      = 0;
    enhance.m_name                          = name;
    enhance.m_level                         = level;
    enhance.m_enhance_info.m_pcat_idx       = pcat_idx;
    enhance.m_enhance_info.m_pset_idx       = pset_idx;
    enhance.m_enhance_info.m_pow_idx        = pow_idx; // always zero

    addEnhancementToChar(cd, enhance);
}

void addEnhancementToChar(CharacterData &cd, const CharacterEnhancement& enh)
{
    for (uint32_t idx = 0; idx < cd.m_enhancements.size(); ++idx)
    {
        if(!cd.m_enhancements[idx].m_slot_used)
        {
            cd.m_enhancements[idx] = enh;
            cd.m_enhancements[idx].m_slot_idx = idx;
            cd.m_enhancements[idx].m_slot_used = true;
            cd.m_has_updated_powers = true; // update client on power status

            qCDebug(logPowers) << "Character received Enhancement:" << idx
                               << enh.m_name
                               << enh.m_level;
            return;
        }
    }

    qCDebug(logPowers) << "Character cannot hold any more enhancements";
}

CharacterEnhancement *getSetEnhancementBySlot(Entity &e, uint32_t pset_idx_in_array, uint32_t pow_idx_in_array, uint32_t eh_slot)
{
    CharacterPower * pow = nullptr;
    pow = &e.m_char->m_char_data.m_powersets[pset_idx_in_array].m_powers[pow_idx_in_array];

    if(pow == nullptr || eh_slot > pow->m_enhancements.size())
        return nullptr;

    qCDebug(logPowers) << "getSetEnhancementBySlot returned" << pow->m_enhancements[eh_slot].m_name << "at" << eh_slot;
    return &pow->m_enhancements[eh_slot];
}

const CharacterEnhancement* getEnhancement(const Entity &ent, uint32_t idx)
{
    const CharacterData& cd = ent.m_char->m_char_data;
    if(idx >= cd.m_enhancements.size())
    {
        qCWarning(logPowers) << "getEnhancement: Invalid enhancement index:" << idx;
        return nullptr;
    }

    const CharacterEnhancement& enh = cd.m_enhancements[idx];
    if(!enh.m_slot_used)
    {
        qCWarning(logPowers) << "getEnhancement: No enhancement at index:" << idx;
        return nullptr;
    }

    return &enh;
}

uint32_t getNumberEnhancements(const CharacterData &cd)
{
    uint32_t count = 0;
    for(const auto & enhancement : cd.m_enhancements)
    {
        if(enhancement.m_slot_used)
            count++;
    }
    return count;
}

uint32_t getMaxNumberEnhancements(const CharacterData &cd)
{
    return cd.m_enhancements.size();
}

void moveEnhancement(CharacterData &cd, uint32_t src_idx, uint32_t dest_idx)
{
    cd.m_enhancements[src_idx].m_slot_idx = dest_idx;
    cd.m_enhancements[dest_idx].m_slot_idx = src_idx;
    std::swap(cd.m_enhancements[src_idx], cd.m_enhancements[dest_idx]);

    qCDebug(logPowers) << "Moving Enhancement from" << src_idx << "to" << dest_idx;
    cd.m_has_updated_powers = true; // update client on power status
}

void setEnhancement(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t src_idx, uint32_t dest_idx)
{
    CharacterPower *tgt_pow = &ent.m_char->m_char_data.m_powersets[pset_idx].m_powers[pow_idx];
    CharacterEnhancement src_eh = ent.m_char->m_char_data.m_enhancements[src_idx];

    // Set Enhancement into Power
    qCDebug(logPowers) << "Set Enhancement" << src_eh.m_name << src_idx << "to" << dest_idx << "for power:"  << pset_idx << pow_idx;
    src_eh.m_slot_idx = dest_idx;
    tgt_pow->m_enhancements[dest_idx] = src_eh;

    // Clear slot in Enhancement Tray
    trashEnhancement(ent.m_char->m_char_data, src_idx);
}

void trashEnhancement(CharacterData &cd, uint32_t eh_idx)
{
    CharacterEnhancement enhance;
    enhance.m_slot_idx = eh_idx;
    cd.m_enhancements[eh_idx] = enhance;
    cd.m_has_updated_powers = true; // update client on power status

    qCDebug(logPowers) << "Remove Enhancement from" << eh_idx;
}

void trashEnhancementInPower(CharacterData &cd, uint32_t pset_idx, uint32_t pow_idx, uint32_t eh_idx)
{
    CharacterEnhancement enhance;
    enhance.m_slot_idx = eh_idx;
    cd.m_powersets[pset_idx].m_powers[pow_idx].m_enhancements[eh_idx] = enhance;
    cd.m_has_updated_powers = true; // update client on power status

    qCDebug(logPowers) << "Remove Enhancement from" << pset_idx << pow_idx << eh_idx;
}

void trashComboEnhancement(CharacterEnhancement &eh, uint32_t eh_idx)
{
    CharacterEnhancement enhance;
    enhance.m_slot_idx = eh_idx;
    eh = enhance;

    qCDebug(logPowers) << "Remove Enhancement from" << eh_idx;
}

void reserveEnhancementSlot(CharacterPower *pow, uint32_t level_purchased)
{
    if(pow->getPowerTemplate().BoostsAllowed.empty())
        return;

    // Modify based upon level
    auto free_boosts_by_level = getGameData().m_pi_schedule.m_FreeBoostSlotsOnPower;
    int lvl_delta = level_purchased - pow->m_level_bought;
    uint32_t count_for_level = getGameData().countForLevel(lvl_delta, free_boosts_by_level);
    uint32_t cur_max_eh_slots = pow->m_total_eh_slots + count_for_level;

    if(cur_max_eh_slots > 5)
        cur_max_eh_slots = 5; // I0 and I1 client cannot handle more than 5 eh slots

    qCDebug(logPowers) << "Total EH Slots + free slots based upon level" << pow->m_total_eh_slots
                       << "lvl_delta:" << lvl_delta
                       << "count_for_level" << count_for_level
                       << "cur_max_eh_slots" << cur_max_eh_slots;

    while(pow->m_enhancements.size() < cur_max_eh_slots)
    {
        CharacterEnhancement eh;
        eh.m_enhance_info = pow->m_power_info;
        eh.m_slot_idx = pow->m_enhancements.size();
        pow->m_enhancements.push_back(eh);
        pow->m_total_eh_slots = pow->m_enhancements.size();

        qCDebug(logPowers) << "Adding empty EH Slot" << eh.m_slot_idx
                           << eh.m_enhance_info.m_pset_idx
                           << eh.m_enhance_info.m_pow_idx;
    }
}

void buyEnhancementSlots(Entity &ent, uint32_t available_slots, std::vector<int> pset_idx, std::vector<int> pow_idx)
{
    if(pow_idx.size() != available_slots)
        qCDebug(logPowers) << "EH vector size does not equal available slots";

    for(uint32_t i = 0; i < available_slots; ++i)
    {
        CharacterPower * pow = getOwnedPowerByVecIdx(ent, pset_idx[i], pow_idx[i]);

        if(pow == nullptr || pow->getPowerTemplate().m_Name.isEmpty())
            qFatal("Cannot find Power for buying enhancement slot: %d %d", pset_idx[i], pow_idx[i]);

        qCDebug(logPowers) << "Power for EH slot:" << i
            << pow->getPowerTemplate().m_Name
            << "indexes" << pow->m_power_info.m_pcat_idx
            << pow->m_power_info.m_pset_idx
            << pow->m_power_info.m_pow_idx;

        //++pow->m_total_eh_slots;
        reserveEnhancementSlot(pow, getLevel(*ent.m_char));
    }
}


float enhancementCombineChances(CharacterEnhancement *eh1, CharacterEnhancement *eh2)
{
    const std::vector<float> *combine_chances;
    int chance_idx = 0;
    int eh_delta = 0;

    eh_delta = eh1->m_num_combines + eh1->m_level - (eh2->m_num_combines + eh2->m_level);
    if(eh_delta >= 0)
        chance_idx = eh_delta;
    else
        chance_idx = -eh_delta;

    qCDebug(logPowers) << "eh_delta" << eh_delta;
    qCDebug(logPowers) << "chance_idx" << chance_idx;

    if(eh1->m_enhance_tpl.parent_StoredPowerSet == eh2->m_enhance_tpl.parent_StoredPowerSet)
        combine_chances = &getGameData().m_combine_same.CombineChances;
    else
        combine_chances = &getGameData().m_combine_chances.CombineChances;

    int chance_count = combine_chances->size();
    qCDebug(logPowers) << "combine_chances size" << chance_count;

    if( chance_idx >= chance_count )
    {
        if( chance_count > 0 )
            return combine_chances->at(chance_count - 1);

        return 0.0f;
    }

    return combine_chances->at(chance_idx);
}

CombineResult combineEnhancements(Entity &ent, const EnhancemenSlotEntry &slot1, const EnhancemenSlotEntry &slot2)
{
    float   chance = 0.0f;
    bool    success = false;
    bool    destroy = false;


    CharacterEnhancement * eh1 = nullptr;
    CharacterEnhancement * eh2 = nullptr;
    eh1 = getSetEnhancementBySlot(ent, slot1.m_pset_idx, slot1.m_pow_idx, slot1.m_eh_idx);

    if(slot2.m_set_in_power == true)
        eh2 = getSetEnhancementBySlot(ent, slot2.m_pset_idx, slot2.m_pow_idx, slot2.m_eh_idx);
    else
        eh2 = &ent.m_char->m_char_data.m_enhancements[slot2.m_eh_idx];

    if(eh1 == nullptr || eh2 == nullptr)
    {
        qCDebug(logPowers) << "Entity:" << ent.name() << "failed to find enhancements to merge";
        return {false,false};
    }

    // get chance
    chance = enhancementCombineChances(eh1, eh2);
    float ran = float(rand()) / float(RAND_MAX);
    qCDebug(logPowers) << "Rand" << ran << "/" << chance;

    if(ran < chance)
        success = true;

    if(eh1->m_level < eh2->m_level)
        destroy = true;

    if(success)
        eh1->m_num_combines++;
    else if(success && destroy)
        eh1 = eh2; // succeeded, but eh1 was lower level: replace it
    else if(!success && !destroy)
        return {success,destroy};

    if(slot2.m_set_in_power == true)
        trashComboEnhancement(*eh2, slot2.m_eh_idx); // trash it anyway, we've already copied it to eh1
    else
        trashEnhancement(ent.m_char->m_char_data, slot2.m_eh_idx);

    ent.m_char->m_char_data.m_has_updated_powers = true; // update client on power status
    return {success,destroy};
}

void dumpEnhancements(CharacterData &cd)
{
    if(getNumberEnhancements(cd) == 0)
        qDebug().noquote() << "This character has no enhancements.";

    for(size_t i = 0; i < cd.m_enhancements.size(); ++i)
    {
        qDebug().noquote() << "Enhancement: " << cd.m_enhancements[i].m_slot_idx
                           << cd.m_enhancements[i].m_name;
        qDebug().noquote() << "  SlotUsed: " << cd.m_enhancements[i].m_slot_used;
        qDebug().noquote() << "  Level: " << cd.m_enhancements[i].m_level;
        qDebug().noquote() << "  NumCombines: " << cd.m_enhancements[i].m_num_combines;
        qDebug().noquote() << "  CategoryIdx: " << cd.m_enhancements[i].m_enhance_info.m_pcat_idx;
        qDebug().noquote() << "  PowerSetIdx: " << cd.m_enhancements[i].m_enhance_info.m_pset_idx;
        qDebug().noquote() << "  PowerIdx: " << cd.m_enhancements[i].m_enhance_info.m_pow_idx;
    }
}

template<class Archive>
void CharacterInspiration::serialize(Archive &archive, uint32_t const version)
{
    if(version != CharacterInspiration::class_version)
    {
        qCritical() << "Failed to serialize CharacterInspiration, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("InspirationInfo", m_insp_info));
    archive(cereal::make_nvp("Name", m_name));
    archive(cereal::make_nvp("Col", m_col));
    archive(cereal::make_nvp("Row", m_row));
    archive(cereal::make_nvp("HasInsp", m_has_insp));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterInspiration)
CEREAL_CLASS_VERSION(CharacterInspiration, CharacterInspiration::class_version)   // register CharacterInspiration struct version

template<class Archive>
void vInspirations::serialize(Archive &archive, uint32_t const version)
{
    if(version != vInspirations::class_version)
    {
        qCritical() << "Failed to serialize vInspirations, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("Inspirations", m_inspirations));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(vInspirations)
CEREAL_CLASS_VERSION(vInspirations, vInspirations::class_version)   // register vInspirations struct version

template<class Archive>
void CharacterEnhancement::serialize(Archive &archive, uint32_t const version)
{
    if(version != CharacterEnhancement::class_version)
    {
        qCritical() << "Failed to serialize CharacterPowerEnhancement, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("EnhancementInfo", m_enhance_info));
    archive(cereal::make_nvp("Index", m_slot_idx));
    archive(cereal::make_nvp("Name", m_name));
    archive(cereal::make_nvp("Level", m_level));
    archive(cereal::make_nvp("NumCombines", m_num_combines));
    archive(cereal::make_nvp("IsUsed", m_slot_used));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterEnhancement)
CEREAL_CLASS_VERSION(CharacterEnhancement, CharacterEnhancement::class_version)   // register CharacterEnhancement struct version

Power_Data CharacterPower::getPowerTemplate() const
{
    return getGameData().get_power_template(m_power_info.m_pcat_idx, m_power_info.m_pset_idx, m_power_info.m_pow_idx);
}

template<class Archive>
void CharacterPower::serialize(Archive &archive, uint32_t const version)
{
    if(version != CharacterPower::class_version)
    {
        qCritical() << "Failed to serialize CharacterPower, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("PowerInfo", m_power_info));
    archive(cereal::make_nvp("Index", m_index));
    archive(cereal::make_nvp("LevelBought", m_level_bought));
    archive(cereal::make_nvp("ActivationState", m_activation_state));
    archive(cereal::make_nvp("NumEnhancements", m_total_eh_slots));
    archive(cereal::make_nvp("IsLimited", m_is_limited));
    archive(cereal::make_nvp("ChargeRemaining", m_charges_remaining));
    archive(cereal::make_nvp("Lifetime", m_lifetime));
    archive(cereal::make_nvp("Enhancements", m_enhancements));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterPower)
CEREAL_CLASS_VERSION(CharacterPower, CharacterPower::class_version)             // register CharacterPower struct version

template<class Archive>
void CharacterPowerSet::serialize(Archive &archive, uint32_t const version)
{
    if(version != CharacterPowerSet::class_version)
    {
        qCritical() << "Failed to serialize CharacterPowerSet, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("Index", m_index));
    archive(cereal::make_nvp("Category", m_category));
    archive(cereal::make_nvp("LevelBought", m_level_bought));
    archive(cereal::make_nvp("Powers", m_powers));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterPowerSet)
CEREAL_CLASS_VERSION(CharacterPowerSet, CharacterPowerSet::class_version) // register CharacterPowerSet struct version

template<class Archive>
void PowerTrayItem::serialize(Archive &archive, uint32_t const version)
{
    if(version != PowerTrayItem::class_version)
    {
        qCritical() << "Failed to serialize PowerTrayItem, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("EntryType", m_entry_type));
    archive(cereal::make_nvp("PowerSetIdx", m_pset_idx));
    archive(cereal::make_nvp("PowerIdx", m_pow_idx));
    archive(cereal::make_nvp("Command", m_command));
    archive(cereal::make_nvp("ShortName", m_short_name));
    archive(cereal::make_nvp("IconName", m_icon_name));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PowerTrayItem)
CEREAL_CLASS_VERSION(PowerTrayItem, PowerTrayItem::class_version)   // register PowerTrayItem class version

template<class Archive>
void PowerTray::serialize(Archive &archive, uint32_t const version)
{
    if(version != PowerTray::class_version)
    {
        qCritical() << "Failed to serialize PowerTray, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("Powers", m_tray_items));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PowerTray)
CEREAL_CLASS_VERSION(PowerTray, PowerTray::class_version)           // register PowerTray class version


template<class Archive>
void PowerTrayGroup::serialize(Archive &archive, uint32_t const version)
{
    if(version != PowerTrayGroup::class_version)
    {
        qCritical() << "Failed to serialize PowerTrayGroup, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("DefaultPowerSet", m_default_pset_idx));
    archive(cereal::make_nvp("DefaultPower", m_default_pow_idx));
    archive(cereal::make_nvp("PowerTrays", m_trays));
}
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PowerTrayGroup)
CEREAL_CLASS_VERSION(PowerTrayGroup, PowerTrayGroup::class_version) // register PowerTrayGroup class version

//! @}
