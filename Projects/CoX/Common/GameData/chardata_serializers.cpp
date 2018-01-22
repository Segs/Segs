#include "chardata_serializers.h"

#include "chardata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"


template<class Archive>
void serialize(Archive & archive, CharacterData &cd)
{
    char buf[128];

    archive(cereal::make_nvp("Level",cd.m_level));
    archive(cereal::make_nvp("CombatLevel",cd.m_combat_level));
    archive(cereal::make_nvp("XP",cd.m_experience_points));
    archive(cereal::make_nvp("Debt",cd.m_experience_debt));
    archive(cereal::make_nvp("PatrolXP",cd.m_experience_patrol));
    archive(cereal::make_nvp("Influence",cd.m_influence));
    archive(cereal::make_nvp("HasTitles",cd.m_has_titles));
    archive(cereal::make_nvp("ThePrefix",cd.m_has_the_prefix));
    auto *titles = &cd.m_titles;
    for(int i=0; i<3; ++i) {
        sprintf(buf,"Title-%d",i);
        archive(cereal::make_nvp(buf,titles[i]));
    }
    archive(cereal::make_nvp("BattleCry",cd.m_battle_cry));
    archive(cereal::make_nvp("Description",cd.m_character_description));
    archive(cereal::make_nvp("AFK",cd.m_afk));
    archive(cereal::make_nvp("AfkMsg",cd.m_afk_msg));
    archive(cereal::make_nvp("LFG",cd.m_lfg));
    archive(cereal::make_nvp("SuperGroupID",cd.m_supergroup_id));
    archive(cereal::make_nvp("Alignment",cd.m_alignment));
    archive(cereal::make_nvp("LastCostumeID",cd.m_last_costume_id));
    archive(cereal::make_nvp("Class",cd.m_class_name));
    archive(cereal::make_nvp("Origin",cd.m_origin_name));
    archive(cereal::make_nvp("MapName",cd.m_mapName));
    archive(cereal::make_nvp("SuperGroupCostume",cd.m_supergroup_costume));
    archive(cereal::make_nvp("UsingSGCostume",cd.m_using_sg_costume));
}

void saveTo(const CharacterData & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"CharacterData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, CharacterData & m);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, CharacterData & m);

void serializeFromDb(CharacterData &data,const QByteArray &src)
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

/*
#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
//......
  std::stringstream ss;
  {
    cereal::JSONOutputArchive archive( ss );
    CharacterData charData;
    archive( charData );
  }
  m_prepared_char_insert.bindValue(":char_data", QByteArray::fromStdString(charData.str()));
*/
