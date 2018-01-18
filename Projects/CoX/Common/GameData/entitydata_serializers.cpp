#include "entitydata_serializers.h"

#include "entitydata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"


template<class Archive>
void serialize(Archive & archive, EntityData &ed)
{
    archive(cereal::make_nvp("OriginIdx",ed.m_origin_idx));
    archive(cereal::make_nvp("ClassIdx",ed.m_class_idx));
    archive(cereal::make_nvp("Type",ed.m_type));
    archive(cereal::make_nvp("Idx",ed.m_idx));
    archive(cereal::make_nvp("dbID",ed.m_db_id));
}

void saveTo(const EntityData & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"EntityData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, EntityData & m);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, EntityData & m);

void serializeFromDb(EntityData &data,const QByteArray &src)
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
