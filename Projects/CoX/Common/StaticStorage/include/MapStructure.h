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
    struct Lod;
    struct Fog;
    struct Beacon;
    struct Sound;
    struct TexReplace;
    struct Omni;
    struct Ambient;
    struct TintColor;
    struct Property;
    struct Group;
    struct Def : public BinReadable
    {
        DECL_READABLE(Def)
        std::vector<Lod *> m_lods;
        std::vector<Fog *> m_fogs;
        std::vector<Beacon *> m_beacons;
        std::vector<Sound *> m_sounds;
        std::vector<TexReplace *> m_texture_replacements;
        std::vector<Omni *> m_omni;
        std::vector<Ambient *> m_ambients;
        std::vector<TintColor *> m_tint_colors;
        std::vector<Property *> m_properties;
        std::vector<Group *> m_groups;

        std::string m_src_name;
        std::string m_obj_name;
        std::string m_type_name;
        u32 m_flags;
        static void build_schema();
    };
    struct Ref : public BinReadable
    {
        DECL_READABLE(Ref)
        std::string m_src_name;
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
    };

    struct TexReplace : public BinReadable
    {
        DECL_READABLE(TexReplace)
        std::string m_name_src;
        std::string m_name_tgt;
        static void build_schema();
    };
    struct Ambient : public BinReadable
    {
        DECL_READABLE(Ambient)
        Color3ub m_color;
        static void build_schema();
    };
    struct Omni : public BinReadable
    {
        DECL_READABLE(Omni)
        float m_val;
        u32 m_flags;
        Color3ub m_color;
        static void build_schema();
    };

    struct Beacon : public BinReadable
    {
        DECL_READABLE(Beacon)
        std::string m_name;
        float m_val; //radius?
        static void build_schema();
    };
    struct Sound : public BinReadable
    {
        DECL_READABLE(Sound)
        std::string m_name;
        float m_a;
        float m_b;
        float m_c;
        u32 m_flags;
        static void build_schema();
    };
    struct Fog : public BinReadable
    {
        DECL_READABLE(Fog)
        float m_a;
        float m_b;
        float m_c;
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };
    struct TintColor : public BinReadable
    {
        DECL_READABLE(TintColor)
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };

    struct Lod : public BinReadable
    {
        DECL_READABLE(Lod)
        float m_far;
        float m_far_fade;
        float m_near;
        float m_near_fade;
        float m_scale;
        static void build_schema();
    };
    struct Property : public BinReadable
    {
        DECL_READABLE(Property)
        std::string m_txt1;
        std::string m_txt2;
        std::string m_txt3;
        static void build_schema();
    };
    struct Group : public BinReadable
    {
        DECL_READABLE(Group)
        std::string m_name; // this is reference to a model name or def name
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
    };
}
struct SceneStorage : public BinReadable
{
    DECL_READABLE(SceneStorage)
    std::vector<MapStructs::Def *> m_defs;
    std::vector<MapStructs::Ref *> m_refs;
    std::vector<MapStructs::Def *> m_root;
    std::string m_scene_file;
    u32 m_version;
    static void build_schema();
    bool has_children()
    {
        return 0!=(m_defs.size()+m_refs.size()+m_root.size());
    }
};
