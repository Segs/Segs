#include "costume_serializers.h"
#include "serialization_common.h"

#include "Common/GameData/costume_definitions.h"
#include "DataStorage.h"

//static Serialization_Template ColorValue_Tokens[3] = {
//    { "", TOKEN_VEC3_LIST, 0},
//    { "\n", TOKEN_END},
//    { ""},
//};
//static Serialization_Template Palette_Tokens[4] = {
//    { "Color", TOKEN_SUB_TABLE, 0, 0xC, ColorValue_Tokens},
//    { "End", TOKEN_END},
//    { "EndPalette", TOKEN_END},
//    { ""},
//};

//static Serialization_Template Info_Tokens[9] = {
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


//static Serialization_Template Mask_Tokens[5] = {
//    { "Name", TOKEN_STRING, 0},
//    { "displayName", TOKEN_STRING, 4},
//    { "End", TOKEN_END},
//    { "EndMask", TOKEN_END},
//    { ""},
//};
//static Serialization_Template GeoSet_Tokens[10] = {
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
//static Serialization_Template BoneSet_Tokens[7] = {
//    { "Name", TOKEN_STRING},
//    { "DisplayName", TOKEN_STRING, 4, 0},
//    { "GeoSet", TOKEN_SUB_TABLE, 0xC, 0x23C, GeoSet_Tokens},
//    { "End", TOKEN_END},
//    { "EndBoneSet", TOKEN_END},
//    { "End", TOKEN_END},
//    { ""},
//};

//static Serialization_Template Region_Tokens[6] = {
//    { "Name", TOKEN_STRING, 0},
//    { "DisplayName", TOKEN_STRING, 4},
//    { "BoneSet", TOKEN_SUB_TABLE, 0xC, 0x10, BoneSet_Tokens},
//    { "EndRegion", TOKEN_END, 0},
//    { "End", TOKEN_END, 0},
//    { ""},
//};
//static Serialization_Template Origin_Tokens[7] = {
//    { "Name", TOKEN_STRING, 0},
//    { "BodyPalette", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_BodyPalette), 4, Palette_Tokens},
//    { "SkinPalette", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_SkinPalette), 4, Palette_Tokens},
//    { "Region", TOKEN_SUB_TABLE, offsetof(CostumeOrigin_Data,m_Region), 0x10, Region_Tokens},
//    { "End", TOKEN_END},
//    { "EndOrigin", TOKEN_END},
//    { "", TOKEN_INVALID},
//};
//static Serialization_Template Costume2_Tokens[5] = {
//    { "Name", TOKEN_STRING, offsetof(Costume2_Data,m_Name)},
//    { "Origin", TOKEN_SUB_TABLE, offsetof(Costume2_Data,m_Origins), 0x10, Origin_Tokens},
//    { "End", TOKEN_END},
//    { "EndBody", TOKEN_END},
//    { ""},
//};
//static Serialization_Template Costumes_Token[2] = {
//    { "Costume", TOKEN_SUB_TABLE, 0, sizeof(Costume2_Data), Costume2_Tokens},
//    { "", TOKEN_INVALID},
//};

//static Serialization_Template TailorCost_Tokens[13] = {
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
//static Serialization_Template TailorCostSet_Tokens[2] = {
//    { "TailorCostSet", TOKEN_SUB_TABLE, 0, 0x2C, TailorCost_Tokens},
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
    QString _name;
    while(s->nesting_name(_name))
    {
        if(_name.compare("GeoSet")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("BoneSet")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("BodyPalette")==0) {
            Pallette_Data nt;
            ok &= loadFrom(s,&nt);
            target->m_BodyPalette.emplace_back(nt);
        } else if(_name.compare("SkinPalette")==0) {
            Pallette_Data nt;
            ok &= loadFrom(s,&nt);
            target->m_SkinPalette.emplace_back(nt);
        } else if(_name.compare("Region")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Origin")==0) {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Info")==0) {
            target->m_Infos.push_back({});
            ok &= loadFrom(s,target->m_Infos.back());
        } else if(_name.compare("Mask")==0) {
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
