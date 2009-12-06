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
    schema_initialized=true;

    ADD_FIELD(""            ,0x206  ,0x0,0x0,0,STR_REF(Def,m_src_name));
    ADD_FIELD("Obj"         ,0x6  ,0x4,0x0,0,STR_REF(Def,m_obj_name));
    ADD_FIELD("Type"        ,0x6  ,0x8,0x0,0,STR_REF(Def,m_type_name));
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
