/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "trick_serializers.h"

#include "anim_definitions.h"
#include "anim_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "trick_definitions.h"
#include "DataStorage.h"

namespace
{
    bool loadFrom(BinStore * s, TextureModifiers & target)
    {
        bool ok=true;
        s->prepare();
        ok &= s->read(target.name);
        ok &= s->read(target.src_file);
        ok &= s->read(target.Gloss);
        ok  &= s->read(target.Surface);
        ok &= s->read(target.Fade);
        ok &= s->read(target.ScaleST0);
        ok &= s->read(target.ScaleST1);
        ok &= s->read(target.Blend);
        ok &= s->read(target.BumpMap);
        ok &= s->read(target.BlendType);
        ok &= s->read(target.Flags);
        ok &= s->prepare_nested(); // will update the file size left
        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore * s, GeometryModifiers & target)
    {
        bool ok=true;
        s->prepare();
        ok &= s->read(target.name);
        ok &= s->read(target.src_name);
        ok &= s->read(target.LodFar);
        ok &= s->read(target.LodFarFade);
        ok &= s->read(target.LodNear);
        ok &= s->read(target.LodNearFade);
        ok &= s->read(target.node._TrickFlags);
        ok &= s->read(target.ObjFlags);
        ok &= s->read(target.GfxFlags);
        ok &= s->read(target.GroupFlags);
        ok &= s->read(target.Sway);
        ok &= s->read(target.Sway_Rotate);
        ok &= s->read(target.AlphaRef);
        ok &= s->read(target.FogDist);
        ok &= s->read(target.node.SortBias);
        ok &= s->read(target.node.ScrollST0);
        ok &= s->read(target.node.ScrollST1);
        ok &= s->read(target.ShadowDist);
        ok &= s->read(target.NightGlow);
        ok &= s->read(target.node.TintColor0);
        ok &= s->read(target.node.TintColor1);
        ok &= s->read(target.ObjTexBias);
        ok &= s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("StAnim"==_name) {
                target.StAnim.emplace_back();
                ok &= loadFrom(s,target.StAnim.back());
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(s->end_encountered());
        return ok;
    }
} // namespace

bool loadFrom(BinStore * s, SceneModifiers &target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Trick"==_name) {
            target.geometry_mods.emplace_back();
            ok &= loadFrom(s,target.geometry_mods.back());
        } else if("Texture"==_name) {
            target.texture_mods.emplace_back();
            ok &= loadFrom(s,target.texture_mods.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, TextureModifiers & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("src_file",m.src_file));
    archive(cereal::make_nvp("Gloss",m.Gloss));
    archive(cereal::make_nvp("Surface",m.Surface));
    archive(cereal::make_nvp("Fade",m.Fade));
    archive(cereal::make_nvp("ScaleST0",m.ScaleST0));
    archive(cereal::make_nvp("ScaleST1",m.ScaleST1));
    archive(cereal::make_nvp("Blend",m.Blend));
    archive(cereal::make_nvp("BumpMap",m.BumpMap));
    archive(cereal::make_nvp("BlendType",m.BlendType));
    archive(cereal::make_nvp("Flags",m.Flags));
}

template<class Archive>
static void serialize(Archive & archive, ModelModifiers & m)
{
    archive(cereal::make_nvp("TrickFlags",m._TrickFlags));
    archive(cereal::make_nvp("SortBias",m.SortBias));
    archive(cereal::make_nvp("ScrollST0",m.ScrollST0));
    archive(cereal::make_nvp("ScrollST1",m.ScrollST1));
    archive(cereal::make_nvp("TintColor0",m.TintColor0));
    archive(cereal::make_nvp("TintColor1",m.TintColor1));
}

template<class Archive>
static void serialize(Archive & archive, GeometryModifiers & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("SrcName",m.src_name));
    archive(cereal::make_nvp("LodFar",m.LodFar));
    archive(cereal::make_nvp("LodFarFade",m.LodFarFade));
    archive(cereal::make_nvp("LodNear",m.LodNear));
    archive(cereal::make_nvp("LodNearFade",m.LodNearFade));
    archive(cereal::make_nvp("ObjFlags",m.ObjFlags));
    archive(cereal::make_nvp("GfxFlags",m.GfxFlags));
    archive(cereal::make_nvp("GroupFlags",m.GroupFlags));
    archive(cereal::make_nvp("Sway",m.Sway));
    archive(cereal::make_nvp("Sway_Rotate",m.Sway_Rotate));
    archive(cereal::make_nvp("AlphaRef",m.AlphaRef));
    archive(cereal::make_nvp("FogDist",m.FogDist));
    archive(cereal::make_nvp("ShadowDist",m.ShadowDist));
    archive(cereal::make_nvp("NightGlow",m.NightGlow));
    archive(cereal::make_nvp("ObjTexBias",m.ObjTexBias));
    archive(cereal::make_nvp("Node",m.node));
    archive(cereal::make_nvp("TextureAnims",m.StAnim));
}

template<class Archive>
static void serialize(Archive & archive, SceneModifiers & m)
{
    archive(cereal::make_nvp("Texture",m.texture_mods));
    archive(cereal::make_nvp("Geometry",m.geometry_mods));
}

void saveTo(const SceneModifiers &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"Tricks",baseName,text_format);
}
bool loadFrom(const QString &filepath, SceneModifiers &target)
{
    QFSWrapper wrap;
    return commonReadFrom(wrap,filepath,"Tricks",target);
}
bool LoadModifiersData(const QString &fname, SceneModifiers &m)
{
    QFSWrapper wrap;
    BinStore binfile;

    if(fname.contains(".crl"))
    {
        if(!loadFrom(fname, m))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    bool opened = binfile.open(wrap, fname, tricks_i0_requiredCrc);
    if(!opened)
        opened = binfile.open(wrap, fname, tricks_i2_requiredCrc);
    if(!opened)
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if(!loadFrom(&binfile, m))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
}
//! @}
