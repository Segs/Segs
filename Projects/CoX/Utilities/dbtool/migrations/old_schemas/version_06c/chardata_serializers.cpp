/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "chardata_serializers.h"
#include "chardata_definitions.h"
#include "serialization_common.h"

#include "DataStorage.h"
#include "trays_serializers.h"
#include "attrib_serializers.h"

#include "Logging.h"

const constexpr uint32_t PowerPool_Info::class_version;
const constexpr uint32_t CharacterInspiration::class_version;
const constexpr uint32_t CharacterPowerEnhancement::class_version;
const constexpr uint32_t CharacterPower::class_version;
const constexpr uint32_t CharacterPowerSet::class_version;
const constexpr uint32_t Friend::class_version;
const constexpr uint32_t FriendsList::class_version;
const constexpr uint32_t Sidekick::class_version;
const constexpr uint32_t CharacterData::class_version;
CEREAL_CLASS_VERSION(PowerPool_Info, PowerPool_Info::class_version)   // register PowerPool_Info class version
CEREAL_CLASS_VERSION(CharacterInspiration, CharacterInspiration::class_version)   // register CharacterInspiration struct version
CEREAL_CLASS_VERSION(CharacterPowerEnhancement, CharacterPowerEnhancement::class_version)   // register CharacterPowerEnhancement struct version
CEREAL_CLASS_VERSION(CharacterPower, CharacterPower::class_version)             // register CharacterPower struct version
CEREAL_CLASS_VERSION(CharacterPowerSet, CharacterPowerSet::class_version)       // register CharacterPowerSet struct version
CEREAL_CLASS_VERSION(Friend, Friend::class_version)                 // register Friend struct version
CEREAL_CLASS_VERSION(FriendsList, FriendsList::class_version)       // register FriendList struct version
CEREAL_CLASS_VERSION(Sidekick, Sidekick::class_version)             // register Sidekick struct version
CEREAL_CLASS_VERSION(CharacterData, CharacterData::class_version)   // register CharacterData struct version

template<class Archive>
void serialize(Archive &archive, PowerPool_Info &poolinfo, uint32_t const version)
{
    if (version != PowerPool_Info::class_version)
    {
        qCritical() << "Failed to serialize PowerPool_Info, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("CategoryIdx", poolinfo.m_category_idx));
    archive(cereal::make_nvp("PowersetEntryIdx", poolinfo.m_pset_idx));
    archive(cereal::make_nvp("PowerIdx", poolinfo.m_pow_idx));
}

template<class Archive>
void serialize(Archive &archive, CharacterInspiration &in, uint32_t const version)
{
    if (version != CharacterInspiration::class_version)
    {
        qCritical() << "Failed to serialize CharacterInspiration, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("InspirationInfo", in.m_insp_tpl));
    archive(cereal::make_nvp("Col", in.m_col));
    archive(cereal::make_nvp("Row", in.m_row));
    archive(cereal::make_nvp("HasInsp", in.m_has_insp));
}

template<class Archive>
void serialize(Archive &archive, CharacterPowerEnhancement &eh, uint32_t const version)
{
    if (version != CharacterPowerEnhancement::class_version)
    {
        qCritical() << "Failed to serialize CharacterPowerEnhancement, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("EnhancementInfo", eh.m_enhance_tpl));
    archive(cereal::make_nvp("Index", eh.m_enhancement_idx));
    archive(cereal::make_nvp("Level", eh.m_level));
    archive(cereal::make_nvp("NumCombines", eh.m_num_combines));
    archive(cereal::make_nvp("IsUsed", eh.m_is_used));
}

template<class Archive>
void serialize(Archive &archive, CharacterPower &pwr, uint32_t const version)
{
    if (version != CharacterPower::class_version)
    {
        qCritical() << "Failed to serialize CharacterPower, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("PowerInfo", pwr.m_power_tpl));
    archive(cereal::make_nvp("Name", pwr.m_name));
    archive(cereal::make_nvp("LevelBought", pwr.m_level_bought));
    archive(cereal::make_nvp("NumCharges", pwr.m_num_charges));
    archive(cereal::make_nvp("UsageTime", pwr.m_usage_time));
    archive(cereal::make_nvp("ActivationTime", pwr.m_activation_time));
    archive(cereal::make_nvp("Range", pwr.m_range));
    archive(cereal::make_nvp("RechargeTime", pwr.m_recharge_time));
    archive(cereal::make_nvp("ActivationState", pwr.m_activation_state));
    archive(cereal::make_nvp("NumEnhancements", pwr.m_num_enhancements));
    archive(cereal::make_nvp("Enhancements", pwr.m_enhancements));
}

template<class Archive>
void serialize(Archive &archive, CharacterPowerSet &pset, uint32_t const version)
{
    if (version != CharacterPowerSet::class_version)
    {
        qCritical() << "Failed to serialize CharacterPowerSet, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("Index", pset.m_pset_idx));
    archive(cereal::make_nvp("LevelBought", pset.m_level_bought));
    archive(cereal::make_nvp("Powers", pset.m_powers));
}

template<class Archive>
void serialize(Archive &archive, Friend &fr, uint32_t const version)
{
    if (version != Friend::class_version)
    {
        qCritical() << "Failed to serialize Friend, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("FriendOnline", fr.m_online_status));
    archive(cereal::make_nvp("FriendDbId", fr.m_db_id));
    archive(cereal::make_nvp("FriendName", fr.m_name));
    archive(cereal::make_nvp("FriendClass", fr.m_class_idx));
    archive(cereal::make_nvp("FriendOrigin", fr.m_origin_idx));
    archive(cereal::make_nvp("FriendMapIdx", fr.m_map_idx));
    archive(cereal::make_nvp("FriendMapname", fr.m_mapname));
}

template<class Archive>
void serialize(Archive &archive, FriendsList &fl, uint32_t const version)
{
    if (version != FriendsList::class_version)
    {
        qCritical() << "Failed to serialize FriendsList, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("HasFriends",fl.m_has_friends));
    archive(cereal::make_nvp("FriendsCount",fl.m_friends_count));
    archive(cereal::make_nvp("Friends",fl.m_friends));
}

template<class Archive>
void serialize(Archive &archive, Sidekick &sk, uint32_t const version)
{
    if (version != Sidekick::class_version)
    {
        qCritical() << "Failed to serialize Sidekick, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("HasSidekick",sk.m_has_sidekick));
    archive(cereal::make_nvp("SidekickDbId",sk.m_db_id));
    archive(cereal::make_nvp("SidekickType",sk.m_type));
}

template<class Archive>
void serialize(Archive &archive, CharacterData &cd, uint32_t const version)
{
    if (version != CharacterData::class_version)
    {
        qCritical() << "Failed to serialize CharacterData, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("Level",cd.m_level));
    archive(cereal::make_nvp("CombatLevel",cd.m_combat_level));
    archive(cereal::make_nvp("XP",cd.m_experience_points));
    archive(cereal::make_nvp("Debt",cd.m_experience_debt));
    archive(cereal::make_nvp("PatrolXP",cd.m_experience_patrol));
    archive(cereal::make_nvp("Influence",cd.m_influence));
    archive(cereal::make_nvp("HasTitles",cd.m_has_titles));
    archive(cereal::make_nvp("ThePrefix",cd.m_has_the_prefix));
    archive(cereal::make_nvp("Titles",cd.m_titles));
    archive(cereal::make_nvp("BattleCry",cd.m_battle_cry));
    archive(cereal::make_nvp("Description",cd.m_character_description));
    archive(cereal::make_nvp("AFK",cd.m_afk));
    archive(cereal::make_nvp("AfkMsg",cd.m_afk_msg));
    archive(cereal::make_nvp("LFG",cd.m_lfg));
    archive(cereal::make_nvp("Alignment",cd.m_alignment));
    archive(cereal::make_nvp("LastCostumeID",cd.m_last_costume_id));
    archive(cereal::make_nvp("LastOnline",cd.m_last_online));
    archive(cereal::make_nvp("Class",cd.m_class_name));
    archive(cereal::make_nvp("Origin",cd.m_origin_name));
    archive(cereal::make_nvp("SuperGroupCostume",cd.m_supergroup_costume));
    archive(cereal::make_nvp("UsingSGCostume",cd.m_using_sg_costume));
    archive(cereal::make_nvp("SideKick",cd.m_sidekick));
    archive(cereal::make_nvp("FriendList",cd.m_friendlist));
    archive(cereal::make_nvp("CurrentAttribs", cd.m_current_attribs));
    archive(cereal::make_nvp("PowerSets", cd.m_powersets));
    archive(cereal::make_nvp("PowerTrayGroups", cd.m_trays));
    archive(cereal::make_nvp("Inspirations", cd.m_inspirations));
    archive(cereal::make_nvp("Enhancements", cd.m_enhancements));
    archive(cereal::make_nvp("MaxInspirationCols", cd.m_max_insp_cols));
    archive(cereal::make_nvp("MaxInspirationRows", cd.m_max_insp_rows));
}

void saveTo(const CharacterData &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"CharacterData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, CharacterData & m, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, CharacterData & m, uint32_t const version);

//! @}
