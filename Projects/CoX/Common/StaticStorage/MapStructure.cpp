/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <ace/ACE.h>
#include "MapStructure.h"

using namespace MapStructs;

DEF_SCHEMA(SceneStorage);
DEF_SCHEMA(Def);
DEF_SCHEMA(Ref);
DEF_SCHEMA(Lod);
DEF_SCHEMA(Fog);
DEF_SCHEMA(Beacon);
DEF_SCHEMA(TexReplace);
DEF_SCHEMA(Sound);
DEF_SCHEMA(Omni);
DEF_SCHEMA(Ambient);
DEF_SCHEMA(TintColor);
DEF_SCHEMA(Property);
DEF_SCHEMA(Group);

void Lod::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field("Far"         ,0xA  ,0x0, 0x0,FLT_REF(Lod,m_far));
    m_schema.add_field("FarFade"     ,0xA  ,0x4, 0x0,FLT_REF(Lod,m_far_fade));
    m_schema.add_field("Near"        ,0xA  ,0x8, 0x0,FLT_REF(Lod,m_near));
    m_schema.add_field("NearFade"    ,0xA  ,0xC, 0x0,FLT_REF(Lod,m_near_fade));
    m_schema.add_field("Scale"       ,0xA  ,0x10,0xC,FLT_REF(Lod,m_scale));
    m_schema.add_end("End");
}

void Sound::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0x6  ,0x0  ,0x0,STR_REF(Sound,m_name));
    m_schema.add_field(""    ,0xA  ,0x0  ,0x0,FLT_REF(Sound,m_a));
    m_schema.add_field(""    ,0xA  ,0x4  ,0x0,FLT_REF(Sound,m_b));
    m_schema.add_field(""    ,0xA  ,0x8  ,0x0,FLT_REF(Sound,m_c));
    m_schema.add_field(""    ,0xF  ,0x10 ,0x0,U32_REF(Def,m_flags)); // bitfield
    m_schema.add_end("\n");
}
void Ambient::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    m_schema.add_field("",0x20D,0xC,0x0,(Color3ub BinReadable::*)&Ambient::m_color);//
    m_schema.add_end("\n");
}

void TintColor::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    m_schema.add_field(""    ,0x20D  ,0x0,0x0,(Color3ub BinReadable::*)&TintColor::m_col1);
    m_schema.add_field(""    ,0x20D  ,0x4,0x0,(Color3ub BinReadable::*)&TintColor::m_col2);
    m_schema.add_end("\n");

}

void Fog::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0xA  ,0x0,0x0,FLT_REF(Fog,m_a));
    m_schema.add_field(""    ,0xA  ,0x4,0x0,FLT_REF(Fog,m_b));
    m_schema.add_field(""    ,0xA  ,0x8,0x0,FLT_REF(Fog,m_c));
    m_schema.add_field(""    ,0xD  ,0xC,0x0,(Color3ub BinReadable::*)&Fog::m_col1);
    m_schema.add_field(""    ,0xD  ,0x10,0x0,(Color3ub BinReadable::*)&Fog::m_col2);
    m_schema.add_end("\n");
}
void Omni::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    m_schema.add_field(""    ,0x20D  ,0x0,0x0,(Color3ub BinReadable::*)&Omni::m_color);
    m_schema.add_field(""    ,0x20A  ,0x4,0x0,FLT_REF(Omni,m_val));
    m_schema.add_field(""    ,0x20F  ,0x8,0x0,U32_REF(Omni,m_flags)); // bitfield
    m_schema.add_end("");
}
void Beacon::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0x206  ,0x0,0x0,STR_REF(Beacon,m_name));
    m_schema.add_field(""    ,0x20A  ,0x4,0x0,FLT_REF(Beacon,m_val));
    m_schema.add_end("\n");
}

void TexReplace::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0x205  ,0x0,0x0,U32_REF(TexReplace,m_src));
    m_schema.add_field(""    ,0x206  ,0x4,0x0,STR_REF(TexReplace,m_name_tgt));
    m_schema.add_end("\n");
}
void Property::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0x206  ,0x0,0x0,STR_REF(Property,m_txt1));
    m_schema.add_field(""    ,0x206  ,0x4,0x0,STR_REF(Property,m_txt2));
    m_schema.add_field(""    ,0x205  ,0x8,0x0,U32_REF(Property,m_val3));
    m_schema.add_end("\n");
}
void Group::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""    ,0x206  ,0x00,0x0,STR_REF(Group,m_name));
    m_schema.add_field("Pos" ,0xC    ,0x04,0xC,VEC3_REF(Group,m_pos));
    m_schema.add_field("Rot" ,0xC    ,0x10,0x8,VEC3_REF(Group,m_rot));
    m_schema.add_end("End");
}
void Ref::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    m_schema.add_field(""            ,0x206  ,0x0,0x0,STR_REF(Ref,m_src_name));
    m_schema.add_field("Pos"         ,0xC   ,0x4,0xC,VEC3_REF(Ref,m_pos));
    m_schema.add_field("Rot"         ,0xC  ,0x10,0x8,VEC3_REF(Ref,m_rot));
    //m_schema.add_end("");
}

void Def::build_schema()
{
    if(schema_initialized)
        return;
    TexReplace::build_schema();
    Omni::build_schema();
    Sound::build_schema();
    Beacon::build_schema();
    Fog::build_schema();
    Ambient::build_schema();
    Lod::build_schema();
    TintColor::build_schema();
    Property::build_schema();
    Group::build_schema();

    schema_initialized=true;
    m_schema.add_field(""   ,0x206  ,0x0    ,0x00     ,STR_REF(Def,m_src_name));
    m_schema.add_field_nested("ReplaceTex"  ,0x15   ,0x1C   ,0x08   ,TARGETED_ARR_OF_REF(Def,TexReplace,m_texture_replacements));
    m_schema.add_field_nested("Omni"        ,0x15   ,0x20   ,0x0C   ,TARGETED_ARR_OF_REF(Def,Omni,m_omni));
    m_schema.add_field_nested("Sound"       ,0x15   ,0x18   ,0x14   ,TARGETED_ARR_OF_REF(Def,Sound,m_sounds));
    m_schema.add_field_nested("Beacon"      ,0x15   ,0x24   ,0x08   ,TARGETED_ARR_OF_REF(Def,Beacon,m_beacons));
    m_schema.add_field_nested("Fog"         ,0x15   ,0x28   ,0x14   ,TARGETED_ARR_OF_REF(Def,Fog,m_fogs));
    m_schema.add_field_nested("Ambient"     ,0x15   ,0x2C   ,0x04   ,TARGETED_ARR_OF_REF(Def,Ambient,m_ambients));
    m_schema.add_field_nested("Lod"         ,0x15   ,0x30   ,0x14   ,TARGETED_ARR_OF_REF(Def,Lod,m_lods));
    m_schema.add_field_nested("TintColor"   ,0x15   ,0x14   ,0x08   ,TARGETED_ARR_OF_REF(Def,TintColor,m_tint_colors));
    m_schema.add_field_nested("Property"    ,0x15   ,0x10   ,0x0C   ,TARGETED_ARR_OF_REF(Def,Property,m_properties));
    m_schema.add_field_nested("Group"       ,0x15   ,0x0C   ,0x1C   ,TARGETED_ARR_OF_REF(Def,Group,m_groups));
    m_schema.add_field("Type" ,0x6    ,0x08   ,0x00     ,STR_REF(Def,m_type_name));
    m_schema.add_field("Flags",0xF    ,0x34   ,0x00     ,U32_REF(Def,m_flags)); // bitfield
    m_schema.add_field("Obj"  ,0x6    ,0x04   ,0x00     ,STR_REF(Def,m_obj_name));
    m_schema.add_end("End");
    m_schema.add_end("DefEnd");
    //m_schema.add_end("");
}

void SceneStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    Def::build_schema();
    Ref::build_schema();

    m_schema.add_field("Version"            ,0x5    ,0x0,0x0,U32_REF(SceneStorage,m_version));
    m_schema.add_field("Scenefile"          ,0x6    ,0xC,0x0,STR_REF(SceneStorage,m_scene_file));
    m_schema.add_field_nested("Def"         ,0x15   ,0x4,0x8 ,TARGETED_ARR_OF_REF(SceneStorage,Def,m_defs));
    m_schema.add_field_nested("RootMod"     ,0x115  ,0x4,0x8 ,TARGETED_ARR_OF_REF(SceneStorage,Def,m_root));
    m_schema.add_field_nested("Ref"         ,0x15   ,0x8,0x1C,TARGETED_ARR_OF_REF(SceneStorage,Ref,m_refs));
    m_schema.add_end("");
}
