/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <algorithm>
#include "types.h"
#include "CoXHash.h"
#include "DataStorage.h"
#include "ReadableStructures.h"

struct TreeStore : public BinReadable
{
    virtual size_t num_children() const
    {
        return 0;
    }
    virtual TreeStore *nth_child(size_t )
    {
        return 0;
    }
    virtual size_t idx_of_child(TreeStore *) const
    {
        return ~0UL;
    }
    virtual std::string to_string() const
    {
        return "";
    }
};


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
    struct Def : public TreeStore
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
        virtual std::string to_string() const
        {
            return "Def["+m_src_name+"]";
        }
        virtual size_t num_children() const
        {
            return m_groups.size();
        }
        virtual TreeStore *nth_child(size_t idx)
        {
            return (TreeStore *)m_groups[idx];
        }

    };
    struct Ref : public TreeStore
    {
        DECL_READABLE(Ref)
        std::string m_src_name;
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
        virtual std::string to_string() const
        {
            return "Ref["+m_src_name+"]";
        }
    };

    struct TexReplace : public TreeStore
    {
        DECL_READABLE(TexReplace)
        u32 m_src;
        std::string m_name_tgt;
        static void build_schema();
    };
    struct Ambient : public TreeStore
    {
        DECL_READABLE(Ambient)
        Color3ub m_color;
        static void build_schema();
    };
    struct Omni : public TreeStore
    {
        DECL_READABLE(Omni)
        float m_val;
        u32 m_flags;
        Color3ub m_color;
        static void build_schema();
    };

    struct Beacon : public TreeStore
    {
        DECL_READABLE(Beacon)
        std::string m_name;
        float m_val; //radius?
        static void build_schema();
    };
    struct Sound : public TreeStore
    {
        DECL_READABLE(Sound)
        std::string m_name;
        float m_a;
        float m_b;
        float m_c;
        u32 m_flags;
        static void build_schema();
    };
    struct Fog : public TreeStore
    {
        DECL_READABLE(Fog)
        float m_a;
        float m_b;
        float m_c;
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };
    struct TintColor : public TreeStore
    {
        DECL_READABLE(TintColor)
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };

    struct Lod : public TreeStore
    {
        DECL_READABLE(Lod)
        float m_far;
        float m_far_fade;
        float m_near;
        float m_near_fade;
        float m_scale;
        static void build_schema();
    };
    struct Property : public TreeStore
    {
        DECL_READABLE(Property)
        std::string m_txt1;
        std::string m_txt2;
        u32 m_val3;
        static void build_schema();
    };
    struct Group : public TreeStore
    {
        DECL_READABLE(Group)
        std::string m_name; // this is reference to a model name or def name
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
    };
}
struct SceneStorage : public TreeStore
{
    DECL_READABLE(SceneStorage)
    typedef std::vector<MapStructs::Def *> vDef;
    vDef m_defs;
    std::vector<MapStructs::Ref *> m_refs;
    vDef m_root;
    std::string m_scene_file;
    u32 m_version;
    static void build_schema();
    size_t num_children() const
    {
        return m_defs.size()+m_refs.size()+m_root.size();
    }
    virtual TreeStore *nth_child(size_t idx)
    {
        if(idx<m_defs.size())
            return m_defs[idx];
        idx-=m_defs.size();
        if(idx<m_refs.size())
            return m_refs[idx];
        idx-=m_refs.size();
        if(idx<m_root.size())
            return m_root[idx];
        return 0;
    }
    virtual size_t idx_of_child(TreeStore *child)
    {
        vDef::iterator itr=std::find(m_defs.begin(),m_defs.end(),child);
        if(itr!=m_defs.end())
            return itr-m_defs.begin();
        return ~0UL;
    }
    virtual std::string to_string() const
    {
        return m_scene_file;
    }
};
