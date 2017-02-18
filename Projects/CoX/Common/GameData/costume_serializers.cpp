#include "costume_serializers.h"
#include "serialization_common.h"

#include "Common/GameData/costume_definitions.h"
#include "DataStorage.h"

//static CharAttr_Nested1 stru_6EC4F8[3] = {
//    { "", TOKEN_VEC3_LIST, 0},
//    { "\n", TOKEN_END},
//    { ""},
//};
//static CharAttr_Nested1 Palette_Tokens[4] = {
//    { "Color", TOKEN_SUB_TABLE, 0, 0xC, stru_6EC4F8},
//    { "End", TOKEN_END},
//    { "EndPalette", TOKEN_END},
//    { ""},
//};

//static CharAttr_Nested1 Info_Tokens[9] = {
//    { "DisplayName", TOKEN_STRING, 0},
//    { "GeoName", TOKEN_STRING, 8},
//    { "Geo", TOKEN_STRING, 4},
//    { "Tex1", TOKEN_STRING, 0x0C},
//    { "Tex2", TOKEN_STRING, 0x10},
//    { "DevOnly", TOKEN_ENUM, 0x14},
//    { "End", TOKEN_END},
//    { "EndTex", TOKEN_END},
//    { ""},
//};


//static CharAttr_Nested1 Mask_Tokens[5] = {
//    { "Name", TOKEN_STRING, 0},
//    { "displayName", TOKEN_STRING, 4},
//    { "End", TOKEN_END},
//    { "EndMask", TOKEN_END},
//    { ""},
//};
//static CharAttr_Nested1 GeoSet_Tokens[10] = {
//    { "DisplayName", TOKEN_STRING, 4},
//    { "BodyPart", TOKEN_STRING, 8},
//    { "Type", TOKEN_ENUM, 0x0C},
//    { "Info", TOKEN_SUB_TABLE, 0x238, 0x18, Info_Tokens},
//    { "Mask", TOKEN_SUB_TABLE, 0x234, 8, Mask_Tokens},
//    { "Masks", TOKEN_STRING_VECTOR, 0x22C},
//    { "MaskNames", TOKEN_STRING_VECTOR, 0x230},
//    { "End", TOKEN_END, 0},
//    { "EndGeoSet", TOKEN_END, 0},
//    { "", TOKEN_INVALID, 0},
//};
//static CharAttr_Nested1 BoneSet_Tokens[7] = {
//    { "Name", TOKEN_STRING},
//    { "DisplayName", TOKEN_STRING, 4, 0},
//    { "GeoSet", TOKEN_SUB_TABLE, 0xC, 0x23C, GeoSet_Tokens},
//    { "End", TOKEN_END},
//    { "EndBoneSet", TOKEN_END},
//    { "End", TOKEN_END},
//    { ""},
//};

//static CharAttr_Nested1 Region_Tokens[6] = {
//    { "Name", TOKEN_STRING, 0},
//    { "DisplayName", TOKEN_STRING, 4},
//    { "BoneSet", TOKEN_SUB_TABLE, 0xC, 0x10, BoneSet_Tokens},
//    { "EndRegion", TOKEN_END, 0},
//    { "End", TOKEN_END, 0},
//    { ""},
//};
//static CharAttr_Nested1 Origin_Tokens[7] = {
//    { "Name", TOKEN_STRING, 0},
//    { "BodyPalette", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_BodyPalette), 4, Palette_Tokens},
//    { "SkinPalette", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_SkinPalette), 4, Palette_Tokens},
//    { "Region", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_Region), 0x10, Region_Tokens},
//    { "End", TOKEN_END},
//    { "EndOrigin", TOKEN_END},
//    { "", TOKEN_INVALID},
//};
//static CharAttr_Nested1 stru_6EC9F0[5] = {
//    { "Name", TOKEN_STRING, offsetof(Costume2_Data,m_Name)},
//    { "Origin", TOKEN_SUB_TABLE, offsetof(Costume2_Data,m_Origins), 0x10, Origin_Tokens},
//    { "End", TOKEN_END},
//    { "EndBody", TOKEN_END},
//    { ""},
//};
//static CharAttr_Nested1 Costumes_Token[2] = {
//    { "Costume", TOKEN_SUB_TABLE, 0, sizeof(Costume2_Data), stru_6EC9F0},
//    { "", TOKEN_INVALID},
//};

//static CharAttr_Nested1 stru_6EC330[13] = {
//    { "MinLevel", TOKEN_ENUM, 0},
//    { "MaxLevel", TOKEN_ENUM, 4},
//    { "EntryFee", TOKEN_ENUM, 8},
//    { "Global", TOKEN_ENUM, 0x0C},
//    { "HeadCost", TOKEN_ENUM, 0x10},
//    { "HeadSubCost", TOKEN_ENUM, 0x14},
//    { "UpperCost", TOKEN_ENUM, 0x18},
//    { "UpperSubCost", TOKEN_ENUM, 0x1C},
//    { "LowerCost", TOKEN_ENUM, 0x20},
//    { "LoserSubCost", TOKEN_ENUM, 0x24},
//    { "NumCostumes", TOKEN_ENUM, 0x28},
//    { "End", TOKEN_END},
//    { "", TOKEN_INVALID},
//};
//static CharAttr_Nested1 TailCost_Tokens[2] = {
//    { "TailorCostSet", TOKEN_SUB_TABLE, 0, 0x2C, stru_6EC330},
//    { "", TOKEN_INVALID},
//};

namespace {
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
bool loadFrom(BinStore *s,GeoSet_Info_Data *target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target->m_DisplayName);
    ok &= s->read(target->m_GeoName);
    ok &= s->read(target->m_Geo);
    ok &= s->read(target->m_Tex1);
    ok &= s->read(target->m_Tex2);
    ok &= s->read(target->m_DevOnly);
    ok &= s->prepare_nested();
    return ok && s->end_encountered();
}
bool loadFrom(BinStore *s,GeoSet_Mask_Data *target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target->m_Name);
    ok &= s->read(target->m_DisplayName);
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
    QString _name;
    while(s->nesting_name(_name))
    {
        if(_name.compare("GeoSet")==0) {
            GeoSet_Data *nt = new GeoSet_Data;
            ok &= loadFrom(s,nt);
            target->m_GeoSets.push_back(nt);
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("BoneSet")==0) {
            BoneSet_Data *nt = new BoneSet_Data;
            ok &= loadFrom(s,nt);
            target->m_BoneSets.push_back(nt);
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("BodyPalette")==0) {
            Pallette_Data *nt = new Pallette_Data;
            ok &= loadFrom(s,nt);
            target->m_BodyPalette.push_back(nt);
        } else if(_name.compare("SkinPalette")==0) {
            Pallette_Data *nt = new Pallette_Data;
            ok &= loadFrom(s,nt);
            target->m_SkinPalette.push_back(nt);
        } else if(_name.compare("Region")==0) {
            Region_Data *nt = new Region_Data;
            ok &= loadFrom(s,nt);
            target->m_Region.push_back(nt);
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Origin")==0) {
            CostumeOrigin_Data *nt = new CostumeOrigin_Data;
            ok &= loadFrom(s,nt);
            target->m_Origins.push_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
}

template<class Archive>
void serialize(Archive & archive, ColorEntry_Data & m)
{
    archive(cereal::make_nvp("rgb",m.color));
}

template<class Archive>
void serialize(Archive & archive, Pallette_Data & m)
{
    archive(cereal::make_nvp("Colors",m.m_Colors));
}
template<class Archive>
void serialize(Archive & archive, TailorCost_Data & m)
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Info")==0) {
            GeoSet_Info_Data *nt = new GeoSet_Info_Data;
            ok &= loadFrom(s,nt);
            target->m_Infos.push_back(nt);
        } else if(_name.compare("Mask")==0) {
            GeoSet_Mask_Data *nt = new GeoSet_Mask_Data;
            ok &= loadFrom(s,nt);
            target->m_Masks.push_back(nt);
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Color")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("TailorCostSet")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Costume")==0) {
            Costume2_Data *nt = new Costume2_Data;
            ok &= loadFrom(s,nt);
            target->push_back(nt);
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

void saveTo(const CostumeSet_Data & target, const QString & baseName, bool text_format)
{
    assert(false);
}

void saveTo(const Pallette_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"Palette",baseName,text_format);
}

void saveTo(const GeoSet_Data & target, const QString & baseName, bool text_format)
{
    assert(false);
}
