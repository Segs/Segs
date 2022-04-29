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

#include "bodypart_serializers.h"

#include "bodypart_definitions.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "DataStorage.h"


namespace
{
    bool loadFrom(BinStore *s,BodyPart_Data *target)
    {
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
static bool loadFromI24(BinStore *s,BodyPart_Data &target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_SourceFile);
    ok &= s->read(target.m_BoneCount);
    ok &= s->read(target.m_InfluenceCost);
    ok &= s->read(target.m_GeoName);
    ok &= s->read(target.m_TexName);
    ok &= s->read(target.m_BaseName);
    ok &= s->read(target.m_BoneCount);
    ok &= s->read(target.m_DontClear);

    ok &= s->read(target.boneIndices[0]);
    ok &= s->read(target.boneIndices[1]);
    ok &= s->read(target.part_idx);
    return ok;
}

template<class Archive>
void serialize(Archive & archive, BodyPart_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("SourceFile",m.m_SourceFile));
    archive(cereal::make_nvp("BoneCount",m.m_BoneCount));
    archive(cereal::make_nvp("InfluenceCost",m.m_InfluenceCost));
    archive(cereal::make_nvp("GeoName",m.m_GeoName));
    archive(cereal::make_nvp("TexName",m.m_TexName));
    archive(cereal::make_nvp("BaseName",m.m_BaseName));
    archive(cereal::make_nvp("BoneCount",m.m_BoneCount));
    archive(cereal::make_nvp("DontClear",m.m_DontClear));
}

bool loadFrom(BinStore * s, BodyPartsStorage &target)
{
    s->prepare();
    if(s->isI24Data())
    {
        return s->handleI24StructArray(target.m_parts);
    }
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
    {
        return ok;
    }

    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("BodyPart"==_name)
        {
            BodyPart_Data nt;
            ok &= loadFrom(s,&nt);
            target.m_parts.emplace_back(nt);
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }

    assert(ok);
    return ok;
}

void saveTo(const BodyPartsStorage & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target.m_parts,"BodyParts",baseName,text_format);
}

//! @}
