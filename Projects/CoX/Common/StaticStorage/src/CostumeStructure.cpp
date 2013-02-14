/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/ACE.h>
#include "CostumeStructure.h"

DEF_SCHEMA(CostumeStorage);
DEF_SCHEMA(CostumeEntry);
DEF_SCHEMA(OriginEntry);
DEF_SCHEMA(RegionEntry);
DEF_SCHEMA(BoneSetEntry);
DEF_SCHEMA(GeoSetEntry);
DEF_SCHEMA(GeoSetInfoEntry);
DEF_SCHEMA(GeoSetMaskEntry);

void CostumeEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    OriginEntry::build_schema();

    m_schema.add_field("Name"        ,0x6 ,0x0,0x0,STR_REF(CostumeEntry,m_name));
    m_schema.add_field_nested("Origin",0x15,0x4,0x10,TARGETED_ARR_OF_REF(CostumeEntry,OriginEntry,m_origins));
    m_schema.add_end("End");
    m_schema.add_end("EndBody");
}

void CostumeStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    CostumeEntry::build_schema();

    m_schema.add_field_nested("Costume"      ,0x15,0x0,0x8,TARGETED_ARR_OF_REF(CostumeStorage,CostumeEntry,m_costumes));
}

void OriginEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ColorStorage::build_schema();
    RegionEntry::build_schema();

    m_schema.add_field("Name"        ,0x6 ,0x0,0x0 ,STR_REF(OriginEntry,m_name));
    m_schema.add_field_nested("BodyPalette" ,0x15,0x4,0x4 ,TARGETED_ARR_OF_REF(OriginEntry,ColorStorage,m_body_palette));
    m_schema.add_field_nested("SkinPalette" ,0x15,0x8,0x4 ,TARGETED_ARR_OF_REF(OriginEntry,ColorStorage,m_skin_palette));
    m_schema.add_field_nested("Region"      ,0x15,0xC,0x10,TARGETED_ARR_OF_REF(OriginEntry,RegionEntry,m_regions));
    m_schema.add_end("End");
    m_schema.add_end("EndOrigin");
}
void RegionEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    BoneSetEntry::build_schema();

    m_schema.add_field("Name"        ,0x6 ,0x0,0x0,STR_REF(RegionEntry,m_name));
    m_schema.add_field("DisplayName" ,0x6 ,0x4,0x0,STR_REF(RegionEntry,m_display_name));
    m_schema.add_field_nested("BoneSet"     ,0x15,0xC,0x10,TARGETED_ARR_OF_REF(RegionEntry,BoneSetEntry,m_bonsets));
    m_schema.add_end("End");
    m_schema.add_end("EndRegion");
}

void BoneSetEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    GeoSetEntry::build_schema();

    m_schema.add_field("Name"        ,0x6 ,0x0,0x0,STR_REF(BoneSetEntry,m_name));
    m_schema.add_field("DisplayName" ,0x6 ,0x4,0x0,STR_REF(BoneSetEntry,m_display_name));
    m_schema.add_field_nested("GeoSet"      ,0x15,0xC,0x23C,TARGETED_ARR_OF_REF(BoneSetEntry,GeoSetEntry,m_geoset));
    m_schema.add_end("End");
    m_schema.add_end("EndBoneSet");
}
void GeoSetEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    GeoSetMaskEntry::build_schema();
    GeoSetInfoEntry::build_schema();

    m_schema.add_field("DisplayName" ,0x6 ,0x4,0x0,STR_REF(GeoSetEntry,m_display_name));
    m_schema.add_field("BodyPart"    ,0x6 ,0x8,0x0,STR_REF(GeoSetEntry,m_body_part));
    m_schema.add_field("Type"        ,0x5 ,0xC,0x0,U32_REF(GeoSetEntry,m_typename));

    m_schema.add_field_nested("Info" ,0x15,0x238,0x18,TARGETED_ARR_OF_REF(GeoSetEntry,GeoSetInfoEntry,m_mask_infos));
    m_schema.add_field_nested("Mask" ,0x15,0x234,0x8,TARGETED_ARR_OF_REF(GeoSetEntry,GeoSetMaskEntry,m_mask_vals));
    m_schema.add_field("Masks"       ,0x12,0x22C,0,(std::vector<std::string>  BinReadable::*)&GeoSetEntry::m_masks);
    m_schema.add_field("MaskNames"   ,0x12,0x230,0,(std::vector<std::string>  BinReadable::*)&GeoSetEntry::m_mask_names);
    m_schema.add_end("End");
    m_schema.add_end("EndGeoSet");
}

void GeoSetInfoEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    m_schema.add_field("DisplayName" ,0x6 ,0x0,0x0,STR_REF(GeoSetInfoEntry,m_display_name));
    m_schema.add_field("GeoName"     ,0x6 ,0x8,0x0,STR_REF(GeoSetInfoEntry,m_geo_name));
    m_schema.add_field("Geo"         ,0x6 ,0x4,0x0,STR_REF(GeoSetInfoEntry,m_geo));
    m_schema.add_field("Tex1"        ,0x6 ,0xC,0x0,STR_REF(GeoSetInfoEntry,m_tex1));
    m_schema.add_field("Tex2"        ,0x6 ,0x10,0x0,STR_REF(GeoSetInfoEntry,m_tex2));
    m_schema.add_field("DevOnly"     ,0x5 ,0x14,0x0,U32_REF(GeoSetInfoEntry,m_devonly));
    m_schema.add_end("End");
    m_schema.add_end("EndTex");
}

void GeoSetMaskEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    m_schema.add_field("Name"        ,0x6 ,0x0,0x0,STR_REF(GeoSetMaskEntry,m_name));
    m_schema.add_field("DisplayName" ,0x6 ,0x4,0x0,STR_REF(GeoSetMaskEntry,m_display_name));
    m_schema.add_end("End");
    m_schema.add_end("EndMask");
}

