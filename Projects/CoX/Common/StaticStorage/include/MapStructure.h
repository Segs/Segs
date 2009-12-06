/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include "types.h"
#include "CoXHash.h"
#include "DataStorage.h"
#include "ReadableStructures.h"
namespace MapStructs
{
    struct Def : public BinReadable
    {
        DECL_READABLE(Def);
        std::vector<BinReadable *> m_defs;
        std::vector<BinReadable *> m_refs;
        std::vector<BinReadable *> m_root;
        std::string m_src_name;
        std::string m_obj_name;
        std::string m_type_name;
        static void build_schema();
    };
    struct Ref : public BinReadable
    {
        DECL_READABLE(Ref);
        std::string m_src_name;
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
    };
}
struct SceneStorage : public BinReadable
{
    DECL_READABLE(SceneStorage);
    std::vector<BinReadable *> m_defs;
    std::vector<BinReadable *> m_refs;
    std::vector<BinReadable *> m_root;
    std::string m_scene_file;
    u32 m_version;
    static void build_schema();
};
