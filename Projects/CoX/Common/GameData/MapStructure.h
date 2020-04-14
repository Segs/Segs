/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <algorithm>
#include "CoXHash.h"
#include "DataStorage.h"

struct TreeStore
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
        uint32_t m_flags;
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
        uint32_t m_src;
        std::string m_name_tgt;
        static void build_schema();
    };
    struct Ambient : public TreeStore
    {
        Color3ub m_color;
        static void build_schema();
    };
    struct Omni : public TreeStore
    {
        float m_val;
        uint32_t m_flags;
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
        uint32_t m_flags;
        static void build_schema();
    };
    struct Fog : public TreeStore
    {
        float m_a;
        float m_b;
        float m_c;
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };
    struct TintColor : public TreeStore
    {
        Color3ub m_col1;
        Color3ub m_col2;
        static void build_schema();
    };

    struct Lod : public TreeStore
    {
        float m_far;
        float m_far_fade;
        float m_near;
        float m_near_fade;
        float m_scale;
        static void build_schema();
    };
    struct Property : public TreeStore
    {
        std::string m_txt1;
        std::string m_txt2;
        uint32_t m_val3;
        static void build_schema();
    };
    struct Group : public TreeStore
    {
        std::string m_name; // this is reference to a model name or def name
        Vec3 m_pos;
        Vec3 m_rot;
        static void build_schema();
    };
}
struct SceneStorage : public TreeStore
{
    typedef std::vector<MapStructs::Def *> vDef;
    typedef std::vector<MapStructs::Ref *> vRef;
    vDef m_defs;
    vRef m_refs;
    vDef m_root;
    std::string m_scene_file;
    uint32_t m_version;
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
    virtual size_t idx_of_child(TreeStore *child) const
    {
        vDef::const_iterator itr=std::find(m_defs.begin(),m_defs.end(),child);
        if(itr!=m_defs.end())
            return itr-m_defs.begin();
        return ~0UL;
    }
    virtual std::string to_string() const
    {
        return m_scene_file;
    }
};
