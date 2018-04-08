#include "bodypart_serializers.h"
#include "serialization_common.h"
#include "bodypart_definitions.h"

#include "DataStorage.h"

//Serialization_Template BodyPart_Tokens[11] = {
//    { "Name", TOKEN_STRING, 0, 0},
//    { "BoneCount", TOKEN_ENUM, 0x10, 0},
//    { "InfluenceCost", TOKEN_ENUM, 0x14},
//    { "GeoName", TOKEN_STRING, 0x18, 0},
//    { "", TOKEN_ENUM, 4, 0},
//    { "", TOKEN_ENUM, 8, 0},
//    { "TexName", TOKEN_STRING, 0x1C, 0},
//    { "BaseName", TOKEN_STRING, 0x20, 0},
//    { "BoneCount", TOKEN_ENUM, 0x10, 0},
//    { "End", TOKEN_END},
//    { },
//};
//Serialization_Template AllBodyParts_Tokens[2] = {
//    { "BodyPart", TOKEN_SUB_TABLE, 0, 36, BodyPart_Tokens },
//    {0}
//};

namespace {
bool loadFrom(BinStore *s,BodyPart_Data *target) {
    bool ok = true;
    s->prepare();
    ok &= s->read(target->m_Name);
    ok &= s->read(target->m_BoneCount);
    ok &= s->read(target->m_InfluenceCost);
    ok &= s->read(target->m_GeoName);
    ok &= s->read(target->boneIndices[0]);
    ok &= s->read(target->boneIndices[1]);
    ok &= s->read(target->m_TexName);
    ok &= s->read(target->m_BaseName);
    int m_BoneCount2;
    ok &= s->read(m_BoneCount2);
    assert(m_BoneCount2==target->m_BoneCount);
    ok &= s->prepare_nested(); // will update the file size left
    return (ok && s->end_encountered());
}
}
template<class Archive>
void serialize(Archive & archive, BodyPart_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("BoneCount",m.m_BoneCount));
    archive(cereal::make_nvp("InfluenceCost",m.m_InfluenceCost));
    archive(cereal::make_nvp("GeoName",m.m_GeoName));
    archive(cereal::make_nvp("boneIndices",m.boneIndices));
    archive(cereal::make_nvp("TexName",m.m_TexName));
    archive(cereal::make_nvp("BaseName",m.m_BaseName));
}

bool loadFrom(BinStore * s, AllBodyParts_Data * target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("BodyPart")==0) {
            BodyPart_Data nt;
            ok &= loadFrom(s,&nt);
            target->emplace_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

void saveTo(const AllBodyParts_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"BodyParts",baseName,text_format);
}
