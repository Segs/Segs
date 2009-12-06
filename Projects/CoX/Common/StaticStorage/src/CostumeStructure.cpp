/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
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

    ADD_FIELD("Name"        ,0x6 ,0x0,0x0,0,STR_REF(CostumeEntry,m_name));
    ADD_FIELD("Origin"      ,0x15,0x4,0x10,&OriginEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&CostumeEntry::m_origins);
    ADD_END("End");
    ADD_END("EndBody");
}

void CostumeStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    CostumeEntry::build_schema();

    ADD_FIELD("Costume"      ,0x15,0x0,0x8,&CostumeEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&CostumeStorage::m_costumes);
}

void OriginEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ColorStorage::build_schema();
    RegionEntry::build_schema();

    ADD_FIELD("Name"        ,0x6 ,0x0,0x0 ,0,STR_REF(OriginEntry,m_name));
    ADD_FIELD("BodyPalette" ,0x15,0x4,0x4 ,&ColorStorage::m_schema,(std::vector<BinReadable *>  BinReadable::*)&OriginEntry::m_body_palette);
    ADD_FIELD("SkinPalette" ,0x15,0x8,0x4 ,&ColorStorage::m_schema,(std::vector<BinReadable *>  BinReadable::*)&OriginEntry::m_skin_palette);
    ADD_FIELD("Region"      ,0x15,0xC,0x10,&RegionEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&OriginEntry::m_regions);
    ADD_END("End");
    ADD_END("EndOrigin");
}
void RegionEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    BoneSetEntry::build_schema();

    ADD_FIELD("Name"        ,0x6 ,0x0,0x0,0,STR_REF(RegionEntry,m_name));
    ADD_FIELD("DisplayName" ,0x6 ,0x4,0x0,0,STR_REF(RegionEntry,m_display_name));
    ADD_FIELD("BoneSet"     ,0x15,0xC,0x10,&BoneSetEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&RegionEntry::m_bonsets);
    ADD_END("End");
    ADD_END("EndRegion");
}

void BoneSetEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    GeoSetEntry::build_schema();

    ADD_FIELD("Name"        ,0x6 ,0x0,0x0,0,STR_REF(BoneSetEntry,m_name));
    ADD_FIELD("DisplayName" ,0x6 ,0x4,0x0,0,STR_REF(BoneSetEntry,m_display_name));
    ADD_FIELD("GeoSet"      ,0x15,0xC,0x23C,&GeoSetEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&BoneSetEntry::m_geoset);
    ADD_END("End");
    ADD_END("EndBoneSet");
}
void GeoSetEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    GeoSetMaskEntry::build_schema();
    GeoSetInfoEntry::build_schema();

    ADD_FIELD("DisplayName" ,0x6 ,0x4,0x0,0,STR_REF(GeoSetEntry,m_display_name));
    ADD_FIELD("BodyPart"    ,0x6 ,0x8,0x0,0,STR_REF(GeoSetEntry,m_body_part));
    ADD_FIELD("Type"        ,0x5 ,0xC,0x0,0,U32_REF(GeoSetEntry,m_typename));

    ADD_FIELD("Info"        ,0x15,0x238,0x18,&GeoSetInfoEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&GeoSetEntry::m_mask_infos);
    ADD_FIELD("Mask"        ,0x15,0x234,0x8,&GeoSetMaskEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&GeoSetEntry::m_mask_vals);
    ADD_FIELD("Masks"       ,0x12,0x22C,0,0,(std::vector<std::string>  BinReadable::*)&GeoSetEntry::m_masks);
    ADD_FIELD("MaskNames"   ,0x12,0x230,0,0,(std::vector<std::string>  BinReadable::*)&GeoSetEntry::m_mask_names);
    ADD_END("End");
    ADD_END("EndGeoSet");
}

void GeoSetInfoEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    ADD_FIELD("DisplayName" ,0x6 ,0x0,0x0,0,STR_REF(GeoSetInfoEntry,m_display_name));
    ADD_FIELD("GeoName"     ,0x6 ,0x8,0x0,0,STR_REF(GeoSetInfoEntry,m_geo_name));
    ADD_FIELD("Geo"         ,0x6 ,0x4,0x0,0,STR_REF(GeoSetInfoEntry,m_geo));
    ADD_FIELD("Tex1"        ,0x6 ,0xC,0x0,0,STR_REF(GeoSetInfoEntry,m_tex1));
    ADD_FIELD("Tex2"        ,0x6 ,0x10,0x0,0,STR_REF(GeoSetInfoEntry,m_tex2));
    ADD_FIELD("DevOnly"     ,0x5 ,0x14,0x0,0,U32_REF(GeoSetInfoEntry,m_devonly));
    ADD_END("End");
    ADD_END("EndTex");
}

void GeoSetMaskEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    ADD_FIELD("Name"        ,0x6 ,0x0,0x0,0,STR_REF(GeoSetMaskEntry,m_name));
    ADD_FIELD("DisplayName" ,0x6 ,0x4,0x0,0,STR_REF(GeoSetMaskEntry,m_display_name));
    ADD_END("End");
    ADD_END("EndMask");
}
