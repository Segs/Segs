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

#include "costume_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "Common/GameData/costume_definitions.h"
#include "DataStorage.h"

namespace
{
    bool loadFrom(BinStore *s,TailorCost_Data &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_MinLevel);
        ok &= s->read(target.m_MaxLevel);
        ok &= s->read(target.m_EntryFee);
        ok &= s->read(target.m_Global);
        ok &= s->read(target.m_HeadCost);
        ok &= s->read(target.m_HeadSubCost);
        ok &= s->read(target.m_UpperCost);
        ok &= s->read(target.m_UpperSubCost);
        ok &= s->read(target.m_LowerCost);
        ok &= s->read(target.m_LoserSubCost);
        ok &= s->read(target.m_NumCostumes);
        ok &=  s->prepare_nested(); // will update the file size left
        return ok && s->end_encountered();
    }

    bool loadFrom(BinStore *s,ColorEntry_Data *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->color);
        ok &= s->prepare_nested();
        return ok && s->end_encountered();
    }

    bool loadFrom(BinStore *s,GeoSet_Info_Data &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_DisplayName);
        ok &= s->read(target.m_GeoName);
        ok &= s->read(target.m_Geo);
        ok &= s->read(target.m_Tex1);
        ok &= s->read(target.m_Tex2);
        ok &= s->read(target.m_DevOnly);
        ok &= s->prepare_nested();
        return ok && s->end_encountered();
    }

    bool loadFrom(BinStore *s,GeoSet_Mask_Data &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_Name);
        ok &= s->read(target.m_DisplayName);
        ok &= s->prepare_nested();
        return ok && s->end_encountered();
    }

    bool loadFrom(BinStore *s,BoneSet_Data *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->m_Name);
        ok &= s->read(target->m_Displayname);
        ok &=  s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            if("GeoSet"==_name) {
                target->m_GeoSets.emplace_back();
                ok &= loadFrom(s,&target->m_GeoSets.back());
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }

    bool loadFrom(BinStore *s,Region_Data *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->m_Name);
        ok &= s->read(target->m_Displayname);
        ok &=  s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("BoneSet"==_name) {
                target->m_BoneSets.emplace_back();
                ok &= loadFrom(s,&target->m_BoneSets.back());
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }

    bool loadFrom(BinStore *s,CostumeOrigin_Data *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->m_Name);
        ok &=  s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("BodyPalette"==_name) {
                Pallette_Data nt;
                ok &= loadFrom(s,&nt);
                target->m_BodyPalette.emplace_back(nt);
            } else if("SkinPalette"==_name) {
                Pallette_Data nt;
                ok &= loadFrom(s,&nt);
                target->m_SkinPalette.emplace_back(nt);
            } else if("Region"==_name) {
                Region_Data nt;
                ok &= loadFrom(s,&nt);
                target->m_Region.emplace_back(nt);
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }

    bool loadFrom(BinStore *s,Costume2_Data *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->m_Name);
        ok &=  s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("Origin"==_name) {
                CostumeOrigin_Data nt;
                ok &= loadFrom(s,&nt);
                target->m_Origins.push_back(nt);
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }
} // namespace

template<class Archive>
void serialize(Archive & archive, ColorEntry_Data & m)
{
    archive(cereal::make_nvp("rgb",m.color));
}

template<class Archive>
static void serialize(Archive & archive, Pallette_Data & m)
{
    archive(cereal::make_nvp("Colors",m.m_Colors));
}

template<class Archive>
static void serialize(Archive & archive, TailorCost_Data & m)
{
    archive(cereal::make_nvp("MinLevel",m.m_MinLevel));
    archive(cereal::make_nvp("MaxLevel",m.m_MaxLevel));
    archive(cereal::make_nvp("EntryFee",m.m_EntryFee));
    archive(cereal::make_nvp("Global",m.m_Global));
    archive(cereal::make_nvp("HeadCost",m.m_HeadCost));
    archive(cereal::make_nvp("HeadSubCost",m.m_HeadSubCost));
    archive(cereal::make_nvp("UpperCost",m.m_UpperCost));
    archive(cereal::make_nvp("UpperSubCost",m.m_UpperSubCost));
    archive(cereal::make_nvp("LowerCost",m.m_LowerCost));
    archive(cereal::make_nvp("LoserSubCost",m.m_LoserSubCost));
    archive(cereal::make_nvp("NumCostumes",m.m_NumCostumes));
}

bool loadFrom(BinStore *s,GeoSet_Data *target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target->m_Displayname);
    ok &= s->read(target->m_BodyPart);
    ok &= s->read(target->m_Type);
    ok &= s->read(target->m_MaskStrings);
    ok &= s->read(target->m_MaskNames);
    ok &=  s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Info"==_name) {
            target->m_Infos.push_back({});
            ok &= loadFrom(s,target->m_Infos.back());
        } else if("Mask"==_name) {
            target->m_Masks.push_back({});
            ok &= loadFrom(s,target->m_Masks.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;

}

bool loadFrom(BinStore *s,Pallette_Data *target)
{
    s->prepare();
    bool ok =  s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Color"==_name) {
            ColorEntry_Data nt;
            ok &= loadFrom(s,&nt);
            target->m_Colors.emplace_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, AllTailorCosts_Data * target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("TailorCostSet"==_name) {
            target->emplace_back();
            ok &= loadFrom(s,target->back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, CostumeSet_Data * target)
{
    s->prepare();
    bool ok = s->prepare_nested();
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Costume"==_name) {
            Costume2_Data nt;
            ok &= loadFrom(s,&nt);
            target->emplace_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;

}

void saveTo(const AllTailorCosts_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"TailorCosts",baseName,text_format);
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Mask_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Info_Data & m)
{
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("GeoName",m.m_GeoName));
    archive(cereal::make_nvp("Geo",m.m_Geo));
    archive(cereal::make_nvp("m_Tex1",m.m_Tex1));
    archive(cereal::make_nvp("m_Tex2",m.m_Tex2));
    archive(cereal::make_nvp("DevOnly",m.m_DevOnly));
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Data & m)
{
    archive(cereal::make_nvp("Displayname",m.m_Displayname));
    archive(cereal::make_nvp("BodyPart",m.m_BodyPart));
    archive(cereal::make_nvp("Type",m.m_Type));
    archive(cereal::make_nvp("MaskStrings",m.m_MaskStrings));
    archive(cereal::make_nvp("MaskNames",m.m_MaskNames));
    archive(cereal::make_nvp("Infos",m.m_Infos));
    archive(cereal::make_nvp("Masks",m.m_Masks));
}

template<class Archive>
static void serialize(Archive & archive, BoneSet_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Displayname",m.m_Displayname));
    archive(cereal::make_nvp("GeoSets",m.m_GeoSets));
}

template<class Archive>
static void serialize(Archive & archive, Region_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Displayname",m.m_Displayname));
    archive(cereal::make_nvp("BoneSets",m.m_BoneSets));
}

template<class Archive>
static void serialize(Archive & archive, CostumeOrigin_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("BodyPalette",m.m_BodyPalette));
    archive(cereal::make_nvp("SkinPalette",m.m_SkinPalette));
    archive(cereal::make_nvp("Region",m.m_Region));
}

template<class Archive>
static void serialize(Archive & archive, Costume2_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Origins",m.m_Origins));
}

void saveTo(const CostumeSet_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"Costumes",baseName,text_format);
}

void saveTo(const Pallette_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"Palette",baseName,text_format);
}

void saveTo(const GeoSet_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"GeoSet",baseName,text_format);
}

//! @}
