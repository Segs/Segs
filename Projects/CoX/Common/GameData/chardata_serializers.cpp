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
#include "DataStorage.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "attrib_serializers.h"
#include <QDebug>

const constexpr uint32_t Friend::class_version;
const constexpr uint32_t FriendsList::class_version;
const constexpr uint32_t Sidekick::class_version;
const constexpr uint32_t CharacterData::class_version;
CEREAL_CLASS_VERSION(Friend, Friend::class_version)                // register Friend struct version
CEREAL_CLASS_VERSION(FriendsList, FriendsList::class_version)      // register FriendList struct version
CEREAL_CLASS_VERSION(Sidekick, Sidekick::class_version)            // register Sidekick struct version
CEREAL_CLASS_VERSION(CharacterData, CharacterData::class_version)  // register CharacterData class version

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
}

void saveTo(const CharacterData &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"CharacterData",baseName,text_format);
}

SPECIALIZE_VERSIONED_SERIALIZATIONS(CharacterData);

//! @}
