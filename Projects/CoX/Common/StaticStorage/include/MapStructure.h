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
        std::vector<BinReadable *> m_fogs;
        std::vector<BinReadable *> m_beacons;
        std::vector<BinReadable *> m_sounds;
        std::vector<BinReadable *> m_texture_replacements;
        std::vector<BinReadable *> m_omni;
        std::vector<BinReadable *> m_ambients;
        std::vector<BinReadable *> m_tint_colors;

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

    struct TexReplace : public BinReadable
    {
        DECL_READABLE(TexReplace);
        std::string m_name_src;
        std::string m_name_tgt;
        static void build_schema();
    };
    struct Ambient : public BinReadable
    {
        DECL_READABLE(Ambient);
        Color3ub m_color;
        static void build_schema();
    };
    struct Omni : public BinReadable
    {
        DECL_READABLE(Omni);
        float m_val;
        u32 m_flags;
        Color3ub m_color;
        static void build_schema();
    };

    struct Beacon : public BinReadable
    {
        DECL_READABLE(Beacon);
        std::string m_name;
        float m_val; //radius?
        static void build_schema();
    };
    struct Sound : public BinReadable
    {
        DECL_READABLE(Sound);
        std::string m_name;
        float m_a;
        float m_b;
        float m_c;
        u32 m_flags;
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
    struct TintColor : public BinReadable
    {
        DECL_READABLE(TintColor);
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
