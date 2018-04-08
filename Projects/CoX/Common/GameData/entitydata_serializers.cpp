#include "entitydata_serializers.h"

#include "entitydata_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

#define ENTITY_DATA_CLASS_VERSION 2
CEREAL_CLASS_VERSION(EntityData, ENTITY_DATA_CLASS_VERSION); // register EntityData class version

template<class Archive>
void serialize(Archive & archive, EntityData &ed, uint32_t const version)
{
    if (version == ENTITY_DATA_CLASS_VERSION)
    {
        archive(cereal::make_nvp("AccessLevel",ed.m_access_level));
        archive(cereal::make_nvp("OriginIdx",ed.m_origin_idx));
        archive(cereal::make_nvp("ClassIdx",ed.m_class_idx));
        archive(cereal::make_nvp("Position",ed.pos));
        archive(cereal::make_nvp("Orientation",ed.m_orientation_pyr));
    }
    else
    {
        qCritical() << "Failed to serialize EntityData, incompatible serialization format version " << version;
    }
}

void saveTo(const EntityData & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"EntityData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, EntityData & m, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, EntityData & m, uint32_t const version);

void serializeToDb(const EntityData &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(EntityData &data,const QString &src)
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
