#include "chardata_serializers.h"

#include "chardata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"


template<class Archive>
void serialize(Archive & archive, CharacterData &m)
{
    archive(cereal::make_nvp("Level",m.m_level));
    archive(cereal::make_nvp("Debt",m.m_experience_debt));
    archive(cereal::make_nvp("Patrol XP",m.m_experience_patrol));
    archive(cereal::make_nvp("XP",m.m_experience_points));
    archive(cereal::make_nvp("Influence",m.m_influence));
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
