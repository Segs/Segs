/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/OS.h>
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

void Lod::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD("Far"         ,0xA  ,0x0,0x0,0,FLT_REF(Lod,m_far));
    ADD_FIELD("FarFade"     ,0xA  ,0x4,0x0,0,FLT_REF(Lod,m_far_fade));
    ADD_FIELD("Near"        ,0xA  ,0x8,0x0,0,FLT_REF(Lod,m_near));
    ADD_FIELD("NearFade"    ,0xA  ,0xC,0x0,0,FLT_REF(Lod,m_near_fade));
    ADD_FIELD("Scale"       ,0xA  ,0x10,0xC,0,FLT_REF(Lod,m_scale));
    ADD_END("End");
}

void Sound::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD(""    ,0x6  ,0x0  ,0x0,0,STR_REF(Sound,m_name));
    ADD_FIELD(""    ,0xA  ,0x0  ,0x0,0,FLT_REF(Sound,m_a));
    ADD_FIELD(""    ,0xA  ,0x4  ,0x0,0,FLT_REF(Sound,m_b));
    ADD_FIELD(""    ,0xA  ,0x8  ,0x0,0,FLT_REF(Sound,m_c));
    ADD_FIELD(""    ,0xF  ,0x10 ,0x0,0,U32_REF(Def,m_flags)); // bitfield
    ADD_END("\n");
}

void Fog::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD(""    ,0xA  ,0x0,0x0,0,FLT_REF(Fog,m_a));
    ADD_FIELD(""    ,0xA  ,0x4,0x0,0,FLT_REF(Fog,m_b));
    ADD_FIELD(""    ,0xA  ,0x8,0x0,0,FLT_REF(Fog,m_c));
    ADD_FIELD(""    ,0xD  ,0xC,0x0,0,(Color3ub BinReadable::*)&Fog::m_col1);
    ADD_FIELD(""    ,0xD  ,0x10,0x0,0,(Color3ub BinReadable::*)&Fog::m_col2);
    ADD_END("\n");
}
void Omni::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    ADD_FIELD(""    ,0x20D  ,0x0,0x0,0,(Color3ub BinReadable::*)&Omni::m_color);
    ADD_FIELD(""    ,0x20A  ,0x4,0x0,0,FLT_REF(Omni,m_val));
    ADD_FIELD(""    ,0x20F  ,0x8,0x0,0,U32_REF(Omni,m_flags)); // bitfield
    ADD_END("");
}
void Beacon::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD(""    ,0x206  ,0x0,0x0,0,STR_REF(Beacon,m_name));
    ADD_FIELD(""    ,0x20A  ,0x4,0x0,0,FLT_REF(Beacon,m_val));
    ADD_END("\n");
}

void TexReplace::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD(""    ,0x205  ,0x0,0x0,0,STR_REF(TexReplace,m_name_src));
    ADD_FIELD(""    ,0x206  ,0x4,0x0,0,STR_REF(TexReplace,m_name_tgt));
    ADD_END("\n");
}

void Ref::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ADD_FIELD(""            ,0x206  ,0x0,0x0,0,STR_REF(Ref,m_src_name));
    ADD_FIELD("Pos"         ,0xC   ,0x4,0xC,0,VEC3_REF(Ref,m_pos));
    ADD_FIELD("Rot"         ,0xC  ,0x10,0x8,0,VEC3_REF(Ref,m_rot));
    //ADD_END("");
}

void Def::build_schema()
{
    if(schema_initialized)
        return;
    Lod::build_schema();
    Fog::build_schema();
    Beacon::build_schema();
    TexReplace::build_schema();

    schema_initialized=true;

    ADD_FIELD(""            ,0x206  ,0x0    ,0x00   ,0,STR_REF(Def,m_src_name));
    ADD_FIELD("ReplaceTex"  ,0x15   ,0x1C   ,0x08   ,&TexReplace::m_schema,ARR_REF(Def,m_texture_replacements));
    ADD_FIELD("Omni"        ,0x15   ,0x20   ,0x0C   ,&Omni::m_schema,ARR_REF(Def,m_omni));
    ADD_FIELD("Sound"       ,0x15   ,0x18   ,0x14   ,&Sound::m_schema,ARR_REF(Def,m_sounds));
    ADD_FIELD("Beacon"      ,0x15   ,0x24   ,0x08   ,&Beacon::m_schema,ARR_REF(Def,m_beacons));
    ADD_FIELD("Fog"         ,0x15   ,0x28   ,0x14   ,&Fog::m_schema,ARR_REF(Def,m_fogs));
    ADD_FIELD("Lod"         ,0x15   ,0x30   ,0x14   ,&Lod::m_schema,ARR_REF(Def,m_lods));
    ADD_FIELD("Flags"       ,0xF    ,0x34   ,0x00   ,0,U32_REF(Def,m_flags)); // bitfield
    ADD_FIELD("Obj"         ,0x6    ,0x04   ,0x00   ,0,STR_REF(Def,m_obj_name));
    ADD_FIELD("Type"        ,0x6    ,0x08   ,0x00   ,0,STR_REF(Def,m_type_name));
    ADD_END("End");
    ADD_END("DefEnd");
    //ADD_END("");
}

void SceneStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    Def::build_schema();
    Ref::build_schema();

    ADD_FIELD("Scenefile"   ,0x6    ,0xC,0x0,0,STR_REF(SceneStorage,m_scene_file));
    ADD_FIELD("Def"         ,0x15   ,0x4,0x8,&Def::m_schema,ARR_REF(SceneStorage,m_defs));
    ADD_FIELD("RootMod"     ,0x115  ,0x4,0x8,&Def::m_schema,ARR_REF(SceneStorage,m_defs));
    ADD_FIELD("Ref"         ,0x15   ,0x8,0x1C,&Ref::m_schema,ARR_REF(SceneStorage,m_refs));

    ADD_END("");
}
