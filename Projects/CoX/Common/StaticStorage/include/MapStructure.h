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
        std::vector<BinReadable *> m_lods;
        std::vector<BinReadable *> m_refs;
        std::vector<BinReadable *> m_root;
        std::string m_src_name;
        std::string m_obj_name;
        std::string m_type_name;
        u32 m_flags;
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

    struct Fog : public BinReadable
    {
        DECL_READABLE(Fog);
        float m_a;
        float m_b;
        float m_c;
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };

    struct Lod : public BinReadable
    {
        DECL_READABLE(Lod);
        float m_far;
        float m_far_fade;
        float m_near;
        float m_near_fade;
        float m_scale;
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
