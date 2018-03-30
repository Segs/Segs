#include <QDebug>
#include "chardata_serializers.h"

#include "chardata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

CEREAL_CLASS_VERSION(Friend, 2);        // register Friend struct version
CEREAL_CLASS_VERSION(FriendsList, 2);    // register FriendList struct version
CEREAL_CLASS_VERSION(Sidekick, 1);      // register Sidekick struct version
CEREAL_CLASS_VERSION(CharacterData, 5); // register CharacterData class version

template<class Archive>
void serialize(Archive &archive, Friend &fr, uint32_t const version)
{
    if(version < 2)
        qCritical("Please update your CharacterDatabase Friend data.");

    archive(cereal::make_nvp("FriendOnline", fr.fr_online_status));
    archive(cereal::make_nvp("FriendDbId", fr.fr_db_id));
    archive(cereal::make_nvp("FriendName", fr.fr_name));
    archive(cereal::make_nvp("FriendClass", fr.fr_class_idx));
    archive(cereal::make_nvp("FriendOrigin", fr.fr_origin_idx));
    archive(cereal::make_nvp("FriendMapIdx", fr.fr_map_idx));
    archive(cereal::make_nvp("FriendMapname", fr.fr_mapname));
}

template<class Archive>
void serialize(Archive &archive, FriendsList &fl, uint32_t const version)
{
    if(version < 2)
        qCritical("Please update your CharacterDatabase FriendList data.");

    archive(cereal::make_nvp("HasFriends",fl.m_has_friends));
    archive(cereal::make_nvp("Friends",fl.m_friends));
    archive(cereal::make_nvp("FriendsCount",fl.m_friends_count));
}

template<class Archive>
void serialize(Archive &archive, Sidekick &sk, uint32_t const version)
{
    archive(cereal::make_nvp("HasSidekick",sk.sk_has_sidekick));
    archive(cereal::make_nvp("SidekickDbId",sk.sk_db_id));
    archive(cereal::make_nvp("SidekickType",sk.sk_type));
}

template<class Archive>
void serialize(Archive &archive, CharacterData &cd, uint32_t const version)
{
    if(version < 5)
        qCritical("Please update your CharacterDatabase chardata.");

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
    archive(cereal::make_nvp("MapName",cd.m_mapName));
    archive(cereal::make_nvp("SuperGroupCostume",cd.m_supergroup_costume));
    archive(cereal::make_nvp("UsingSGCostume",cd.m_using_sg_costume));
    archive(cereal::make_nvp("SideKick",cd.m_sidekick));
    archive(cereal::make_nvp("FriendList",cd.m_friendlist));
}

void saveTo(const CharacterData &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"CharacterData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, CharacterData & m, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, CharacterData & m, uint32_t const version);

void serializeToDb(const CharacterData &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(CharacterData &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
