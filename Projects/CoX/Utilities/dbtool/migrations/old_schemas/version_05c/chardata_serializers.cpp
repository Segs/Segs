#include "chardata_serializers.h"

#include "chardata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

CEREAL_CLASS_VERSION(CharacterData, CharacterData::class_version); // register CharacterData class version

template<class Archive>
void serialize(Archive & archive, CharacterData &cd, uint32_t const version)
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
    archive(cereal::make_nvp("MapName",cd.m_mapName));
    archive(cereal::make_nvp("SuperGroupCostume",cd.m_supergroup_costume));
    archive(cereal::make_nvp("UsingSGCostume",cd.m_using_sg_costume));
    archive(cereal::make_nvp("CurrentChatChannel",cd.m_cur_chat_channel));
}

void saveTo(const CharacterData & target, const QString & baseName, bool text_format)
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
