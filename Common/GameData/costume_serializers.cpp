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


static bool loadFromI24(BinStore *s,CostumeGeoData &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileName);
    ok &= s->read(target.m_ShieldPos);
    ok &= s->read(target.m_ShieldPYR);
    return ok;
}

static bool loadFromI24(BinStore *s,TailorCost_Data &target)
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
    return ok;
}

static bool loadFromI24(BinStore *s,GeoSet_Mask_Data &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_Keys);
    ok &= s->read(target.m_Product);
    ok &= s->read(target.m_Tag);
    ok &= s->read(target.m_Legacy);
    ok &= s->read(target.m_DevOnly);
    ok &= s->read(target.m_COV);
    ok &= s->read(target.m_COH);
    ok &= s->read(target.m_COHV);
    return ok;
}

static bool loadFromI24(BinStore *s,CostumeFaceScaleSet &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_Head);
    ok &= s->read(target.m_Brow);
    ok &= s->read(target.m_Cheek);
    ok &= s->read(target.m_Chin);
    ok &= s->read(target.m_Cranium);
    ok &= s->read(target.m_Jaw);
    ok &= s->read(target.m_Nose);
    ok &= s->read(target.m_COV);
    ok &= s->read(target.m_COH);
    return ok;
}

static bool loadFromI24(BinStore *s,CostumeSetSet &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileName);
    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_NpcName);
    ok &= s->read(target.m_Keys);
    ok &= s->read(target.m_StoreProductList);
    return ok;
}

static bool loadFromI24(BinStore *s,GeoSet_Info_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_GeoName);
    ok &= s->read(target.m_Geo);
    ok &= s->read(target.m_Tex1);
    ok &= s->read(target.m_Tex2);
    ok &= s->read(target.m_Fx);
    ok &= s->read(target.m_Product);
    ok &= s->read(target.m_Keys);
    ok &= s->read(target.m_Tag);
    ok &= s->read(target.m_Flags);
    ok &= s->read(target.m_DevOnly);
    ok &= s->read(target.m_COV);
    ok &= s->read(target.m_COH);
    ok &= s->read(target.m_COHV);
    ok &= s->read(target.m_IsMask);
    ok &= s->read(target.m_Level);
    ok &= s->read(target.m_Legacy);
    return ok;
}
static bool loadFromI24(BinStore *s,GeoSet_Data &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_BodyPart);
    ok &= s->read(target.m_ColorLink);
    ok &= s->read(target.m_Keys);
    ok &= s->read(target.m_Flags);
    ok &= s->read(target.m_Product);
    ok &= s->read(target.m_Type);
    ok &= s->read(target.m_AnimBits);
    ok &= s->read(target.m_ZoomBits);
    ok &= s->read(target.m_DefaultView);
    ok &= s->read(target.m_ZoomView);
    ok &= s->read(target.m_NumColor);
    ok &= s->read(target.m_NoDisplay);
    ok &= s->handleI24StructArray(target.m_Infos);
    ok &= s->handleI24StructArray(target.m_Masks);
    ok &= s->read(target.m_MaskStrings);
    ok &= s->read(target.m_MaskNames);
    ok &= s->read(target.m_Legacy);
    ok &= s->handleI24StructArray(target.m_Faces);
    ok &= s->read(target.m_COV);
    ok &= s->read(target.m_COH);

    return ok;
}
static bool loadFromI24(BinStore *s,BoneSet_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileName);
    ok &= s->read(target.m_Displayname);
    ok &= s->read(target.m_Keys);
    ok &= s->read(target.m_Product);
    ok &= s->read(target.m_Flags);
    ok &= s->handleI24StructArray(target.m_GeoSets);
    ok &= s->read(target.m_Legacy);
    ok &= s->read(target.m_COH);
    ok &= s->read(target.m_COV);
    return ok;
}

static bool loadFromI24(BinStore *s,Region_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileName);
    ok &= s->read(target.m_Displayname);
    ok &= s->read(target.m_Keys);
    ok &= s->handleI24StructArray(target.m_BoneSets);
    ok &= s->read(target.m_StoreCategory);
    return ok;
}
static bool loadFromI24(BinStore *s,CostumeOrigin_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileName);
    ok &= s->handleI24StructArray(target.m_BodyPalette);
    ok &= s->handleI24StructArray(target.m_SkinPalette);
    ok &= s->handleI24StructArray(target.m_PowerPalette);
    ok &= s->handleI24StructArray(target.m_Region);
    ok &= s->handleI24StructArray(target.m_CostumeSets);
    ok &= s->handleI24StructArray(target.m_GeoData);
    return ok;
}
static bool loadFromI24(BinStore *s,Costume2_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->handleI24StructArray(target.m_Origins);
    return ok;
}

static bool loadFromI24(BinStore *s,ColorEntry_Data &target)
{
    s->prepare();
    return s->read(target.color);
}

template<class Archive>
void serialize(Archive & archive, ColorEntry_Data & m)
{
    archive(cereal::make_nvp("rgb",m.color));
}

template<class Archive>
static void serialize(Archive & archive, Pallette_Data & m)
{
    archive(cereal::make_nvp("Colors",m.m_Colors));
    archive(cereal::make_nvp("Name",m.m_Name));
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
    if(s->isI24Data()) {
        return loadFromI24(s,*target);
    }
    s->prepare();
    bool ok = true;
    ok &= s->read(target->m_DisplayName);
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

bool loadFromI24(BinStore *s, Pallette_Data &target)
{
    bool ok = true;
    s->prepare();
    ok = s->handleI24StructArray(target.m_Colors);
    ok &= s->read(target.m_Name);
    return ok;
}

bool loadFrom(BinStore *s,Pallette_Data *target)
{
    bool ok = true;
    s->prepare();
    if(s->isI24Data()) {
        return loadFromI24(s,*target);
    }
    ok =  s->prepare_nested(); // will update the file size left
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

bool loadFrom(BinStore * s, AllTailorCosts_Data &target)
{
    s->prepare(); // read the size
    if(s->isI24Data()) {
        return s->handleI24StructArray(target);
    }

    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("TailorCostSet"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
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
    if(s->isI24Data()) {
        return s->handleI24StructArray(*target);
    }
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
static void serialize(Archive & archive, CostumeGeoData & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FileName",m.m_FileName));
    archive(cereal::make_nvp("ShieldPos",m.m_ShieldPos));
    archive(cereal::make_nvp("ShieldPYR",m.m_ShieldPYR));
}

template<class Archive>
static void serialize(Archive & archive, CostumeSetSet & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FileName",m.m_FileName));
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("NpcName",m.m_NpcName));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("StoreProductList",m.m_StoreProductList));
}

template<class Archive>
static void serialize(Archive & archive, CostumeFaceScaleSet & m)
{
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("Head",m.m_Head));
    archive(cereal::make_nvp("Brow",m.m_Brow));
    archive(cereal::make_nvp("Cheek",m.m_Cheek));
    archive(cereal::make_nvp("Chin",m.m_Chin));
    archive(cereal::make_nvp("Cranium",m.m_Cranium));
    archive(cereal::make_nvp("Jaw",m.m_Jaw));
    archive(cereal::make_nvp("Nose",m.m_Nose));
    archive(cereal::make_nvp("COV",m.m_COV));
    archive(cereal::make_nvp("COH",m.m_COH));
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Mask_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("Key",m.m_Key));
    archive(cereal::make_nvp("Product",m.m_Product));
    archive(cereal::make_nvp("Tag",m.m_Tag));
    archive(cereal::make_nvp("Legacy",m.m_Legacy));
    archive(cereal::make_nvp("DevOnly",m.m_DevOnly));
    archive(cereal::make_nvp("COV",m.m_COV));
    archive(cereal::make_nvp("COH",m.m_COH));
    archive(cereal::make_nvp("COHV",m.m_COHV));
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Info_Data & m)
{
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("GeoName",m.m_GeoName));
    archive(cereal::make_nvp("Geo",m.m_Geo));
    archive(cereal::make_nvp("Tex1",m.m_Tex1));
    archive(cereal::make_nvp("Tex2",m.m_Tex2));
    archive(cereal::make_nvp("Fx",m.m_Fx));
    archive(cereal::make_nvp("Product",m.m_Product));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("Tag",m.m_Tag));
    archive(cereal::make_nvp("Flags",m.m_Flags));
    archive(cereal::make_nvp("DevOnly",m.m_DevOnly));
    archive(cereal::make_nvp("COV",m.m_COV));
    archive(cereal::make_nvp("COH",m.m_COH));
    archive(cereal::make_nvp("COHV",m.m_COHV));
    archive(cereal::make_nvp("IsMask",m.m_IsMask));
    archive(cereal::make_nvp("Level",m.m_Level));
    archive(cereal::make_nvp("Legacy",m.m_Legacy));
}

template<class Archive>
static void serialize(Archive & archive, GeoSet_Data & m)
{
    archive(cereal::make_nvp("DisplayName",m.m_DisplayName));
    archive(cereal::make_nvp("BodyPart",m.m_BodyPart));
    archive(cereal::make_nvp("ColorLink",m.m_ColorLink));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("Flags",m.m_Flags));
    archive(cereal::make_nvp("Product",m.m_Product));
    archive(cereal::make_nvp("Type",m.m_Type));
    archive(cereal::make_nvp("AnimBits",m.m_AnimBits));
    archive(cereal::make_nvp("ZoomBits",m.m_ZoomBits));
    archive(cereal::make_nvp("DefaultView",m.m_DefaultView));
    archive(cereal::make_nvp("ZoomView",m.m_ZoomView));
    archive(cereal::make_nvp("NumColor",m.m_NumColor));
    archive(cereal::make_nvp("NoDisplay",m.m_NoDisplay));
    archive(cereal::make_nvp("Infos",m.m_Infos));
    archive(cereal::make_nvp("Masks",m.m_Masks));
    archive(cereal::make_nvp("MaskStrings",m.m_MaskStrings));
    archive(cereal::make_nvp("MaskNames",m.m_MaskNames));
    archive(cereal::make_nvp("Legacy",m.m_Legacy));
    archive(cereal::make_nvp("Faces",m.m_Faces));
    archive(cereal::make_nvp("COH",m.m_COH));
    archive(cereal::make_nvp("COV",m.m_COV));
}

template<class Archive>
static void serialize(Archive & archive, BoneSet_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FileName",m.m_FileName));
    archive(cereal::make_nvp("Displayname",m.m_Displayname));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("Product",m.m_Product));
    archive(cereal::make_nvp("Flags",m.m_Flags));
    archive(cereal::make_nvp("GeoSets",m.m_GeoSets));
    archive(cereal::make_nvp("Legacy",m.m_Legacy));
    archive(cereal::make_nvp("COH",m.m_COH));
    archive(cereal::make_nvp("COV",m.m_COV));
}

template<class Archive>
static void serialize(Archive & archive, Region_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FileName",m.m_FileName));
    archive(cereal::make_nvp("Displayname",m.m_Displayname));
    archive(cereal::make_nvp("Keys",m.m_Keys));
    archive(cereal::make_nvp("BoneSets",m.m_BoneSets));
    archive(cereal::make_nvp("StoreCategory",m.m_StoreCategory));
}

template<class Archive>
static void serialize(Archive & archive, CostumeOrigin_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FileName",m.m_FileName));
    archive(cereal::make_nvp("BodyPalette",m.m_BodyPalette));
    archive(cereal::make_nvp("SkinPalette",m.m_SkinPalette));
    archive(cereal::make_nvp("PowerPalette",m.m_PowerPalette));
    archive(cereal::make_nvp("Region",m.m_Region));
    archive(cereal::make_nvp("CostumeSets",m.m_CostumeSets));
    archive(cereal::make_nvp("GeoData",m.m_GeoData));
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
