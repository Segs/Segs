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

#include "npc_serializers.h"
#include "npc_definitions.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "costume_definitions.h"
#include "DataStorage.h"

namespace
{
    bool loadFrom(BinStore *s, Parse_CostumePart &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_Name);
        ok &= s->read(target.m_CP_Geometry);
        ok &= s->read(target.m_Texture1);
        ok &= s->read(target.m_Texture2);
        ok &= s->read(target.m_Color1);
        ok &= s->read(target.m_Color2);
        ok &=s->prepare_nested();
        assert(ok&&s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, Parse_Costume &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_EntTypeFile);
        ok &= s->read(target.m_CostumeFilePrefix);
        ok &= s->read(target.m_Scale);
        ok  &= s->read(target.m_BoneScale);
        ok &= s->read(target.m_SkinColor);
        ok &=s->prepare_nested();
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("CostumePart"==_name) {
                target.m_CostumeParts.emplace_back();
                ok &= loadFrom(s,target.m_CostumeParts.back());
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }

    bool loadFrom(BinStore *s, NPCPower_Desc &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.PowerCategory);
        ok &= s->read(target.PowerSet);
        ok &= s->read(target.Power);
        ok &= s->read(target.Level);
        ok &= s->read(target.Remove);
        ok &=s->prepare_nested();
        assert(ok && s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, Parse_NPC &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_Name);
        ok &= s->read(target.m_DisplayName);
        ok &= s->read(target.m_Class);
        ok &= s->read(target.m_Level);
        ok &= s->read(target.m_Rank);
        ok &= s->read(target.m_XP);
        ok &=s->prepare_nested();
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("Power"==_name) {
                target.m_Powers.emplace_back();
                ok &= loadFrom(s,target.m_Powers.back());
            } else if("Costume"==_name) {
                target.m_Costumes.emplace_back();
                ok &= loadFrom(s,target.m_Costumes.back());
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }
} // namespace

bool loadFrom(BinStore *s, AllNpcs_Data &target)
{
    s->prepare();
    bool ok = s->prepare_nested();
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("NPC"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, Parse_CostumePart & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Color1",m.m_Color1));
    archive(cereal::make_nvp("Color2",m.m_Color2));
    archive(cereal::make_nvp("Texture1",m.m_Texture1));
    archive(cereal::make_nvp("Texture2",m.m_Texture2));
    archive(cereal::make_nvp("CP_Geometry",m.m_CP_Geometry));
}

template<class Archive>
static void serialize(Archive & archive, Parse_Costume & m)
{
    archive(cereal::make_nvp("EntTypeFile",m.m_EntTypeFile));
    archive(cereal::make_nvp("CostumeFilePrefix",m.m_CostumeFilePrefix));
    archive(cereal::make_nvp("BodyType",m.m_BodyType));
    archive(cereal::make_nvp("Scale",m.m_Scale));
    archive(cereal::make_nvp("BoneScale",m.m_BoneScale));
    archive(cereal::make_nvp("SkinColor",m.m_SkinColor));
    archive(cereal::make_nvp("NumParts",m.m_NumParts));
    archive(cereal::make_nvp("CostumeParts",m.m_CostumeParts));
}

template<class Archive>
static void serialize(Archive & archive, NPCPower_Desc & m)
{
    archive(cereal::make_nvp("PowerCategory",m.PowerCategory));
    archive(cereal::make_nvp("PowerSet",m.PowerSet));
    archive(cereal::make_nvp("Power",m.Power));
    archive(cereal::make_nvp("Level",m.Level));
    archive(cereal::make_nvp("Remove",m.Remove));
}

template<class Archive>
static void serialize(Archive & archive, Parse_NPC & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("Rank",m.m_Rank));
    archive(cereal::make_nvp("Class",m.m_Class));
    archive(cereal::make_nvp("Level",m.m_Level));
    archive(cereal::make_nvp("XP",m.m_XP));
    archive(cereal::make_nvp("Powers",m.m_Powers));
    archive(cereal::make_nvp("Costumes",m.m_Costumes));
}

void saveTo(const AllNpcs_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"AllNpcs",baseName,text_format);
}

//! @}
