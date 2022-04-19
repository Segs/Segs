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
    template<typename T>
    static bool loadAsStruct(BinStore * s,T& target) {
        s->prepare();
        assert(s->current_block_size()==sizeof(T)); // partial reads not supported yet
        s->read(target);
        return true; // we disregard partial reads etc.
    }
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
        ok &= s->readEnum(target.BlendType);
        ok &= s->readEnum(target.Flags);
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
    bool loadAdvancedTextureEntry(BinStore * s,AdvancedTextureEntry &target)
    {
        bool ok=true;
        ok &= s->read(target.Name);
        ok &= loadAsStruct(s,target.Scale);
        ok &= s->read(target.Scroll);
        ok &= s->readEnum(target.ScrollType);
        ok &= s->read(target.Swappable);
        return ok;
    }
    bool loadAdvancedTextureEntry(BinStore * s,TextureMaterialGroup &target)
    {
        bool ok=true;
        ok &= loadAdvancedTextureEntry(s,target.Base);
        ok &= loadAdvancedTextureEntry(s,target.Multiply);
        ok &= loadAdvancedTextureEntry(s,target.DualColor);
        ok &= loadAdvancedTextureEntry(s,target.AddGlow);
        ok &= loadAdvancedTextureEntry(s,target.BumpMap);
        return ok;
    }
    bool loadTextureFallback(BinStore * s,TextureFallback &target) {
        bool ok=true;
        s->prepare();
        ok &= loadAsStruct(s,target.ScaleST1); // a mess, field ScaleST0 references a ScaleSt1 field
        ok &= loadAsStruct(s,target.ScaleST0);
        ok &= s->read(target.Base);
        ok &= s->read(target.Blend);
        ok &= s->read(target.BumpMap);
        ok &= s->readEnum(target.BlendType);
        ok &= s->read(target.UseFallback);
        ok &= loadAsStruct(s,target.DiffuseScaleVec);
        ok &= loadAsStruct(s,target.AmbientScaleVec);
        ok &= s->read(target.AmbientMinVec);
        return ok;
    }
} // namespace

static bool loadFromI24(BinStore * s, TextureModifiers & target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.src_file);
    ok &= s->read(target.TimeStamp);
    ok &= s->read(target.name); // name stored twice....
    ok &= s->read(target.Gloss);
    ok &= s->read(target.Surface);
    ok &= s->read(target.Fade);
    ok &= loadAsStruct(s,target.ScaleST0);
    ok &= loadAsStruct(s,target.ScaleST1);
    ok &= s->read(target.Blend);
    ok &= s->read(target.BumpMap);
    ok &= s->readEnum(target.BlendType);
    ok &= s->readEnum(target.Flags);
    ok &= s->read(target.DF_ObjName);
    ok &= loadAdvancedTextureEntry(s,target.Layer1);
    ok &= loadAdvancedTextureEntry(s,target.Mask);
    ok &= loadAdvancedTextureEntry(s,target.Layer2);
    ok &= loadAdvancedTextureEntry(s,target.CubeMap);
    ok &= loadAsStruct(s,target.rgba3);
    ok &= s->read(target.rgba4);
    ok &= loadAsStruct(s,target.Specular1);
    ok &= s->read(target.Specular2);
    ok &= s->read(target.SpecularExp1);
    ok &= s->read(target.SpecularExp2);
    ok &= s->read(target.Reflectivity);
    ok &= s->read(target.ReflectivityBase);
    ok &= s->read(target.ReflectivityScale);
    ok &= s->read(target.ReflectivityPower);
    ok &= s->read(target.AlphaMask);
    ok &= s->read(target.MaskWeight);
    ok &= s->read(target.Multiply1Reflect);
    ok &= s->read(target.Multiply2Reflect);
    ok &= s->read(target.BaseAddGlow);
    ok &= s->read(target.MinAddGlow);
    ok &= s->read(target.MaxAddGlow);
    ok &= s->read(target.AddGlowMat2);
    ok &= s->read(target.AddGlowTint);
    ok &= s->read(target.ReflectionTint);
    ok &= s->read(target.ReflectionDesaturate);
    ok &= s->read(target.AlphaWater);
    ok &= loadAsStruct(s,target.DiffuseScaleVec);
    ok &= loadAsStruct(s,target.AmbientScaleVec);
    ok &= s->read(target.AmbientMinVec);
    ok &= loadTextureFallback(s,target.Fallback);
    return ok;
}


static bool loadFromI24(BinStore * s, AutoLOD & target)
{
    bool ok=true;
    ok &= s->read(target.AllowedError);
    ok &= s->read(target.LodNearFade);
    ok &= s->read(target.LodNear);
    ok &= s->read(target.LodFarFade);
    ok &= s->read(target.LodFar);
    ok &= s->readEnum(target.Lodflags);
    ok &= s->read(target.ModelName);
    s->prepare();

    return ok;
}
static bool loadFromI24(BinStore * s, GeometryModifiers & target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.src_name);
    ok &= s->read(target.m_Timestamp);

    ok &= s->read(target.LodFar);
    ok &= s->read(target.LodFarFade);
    ok &= s->read(target.LodNear);
    ok &= s->read(target.LodNearFade);
    ok &= s->read(target.node._TrickFlags);
    ok &= s->read(target.ObjFlags);
    ok &= s->read(target.GroupFlags);
    ok &= s->read(target.Sway);
    ok &= s->read(target.SwayRandomize);
    ok &= s->read(target.SwayPitch);
    ok &= s->read(target.SwayRoll);
    ok &= s->read(target.AlphaRef);
    ok &= s->read(target.node.SortBias);
    ok &= s->read(target.WaterReflectionSkew);
    ok &= s->read(target.WaterReflectionStrength);
    ok &= s->read(target.node.ScrollST0);
    ok &= s->read(target.node.ScrollST1);
    ok &= s->read(target.ShadowDist);
    ok &= s->read(target.NightGlow);
    ok &= s->read(target.node.TintColor0);
    ok &= s->read(target.node.TintColor1);
    ok &= s->read(target.ObjTexBias);
    ok &= s->read(target.CameraFaceTightenUp);
    ok &= s->handleI24StructArray(target.StAnim);
    ok &= s->handleI24StructArray(target.AutoLOD);
    return ok;
}

bool loadFrom(BinStore * s, SceneModifiers &target)
{
    bool ok = true;
    s->prepare();
    if(s->isI24Data()) {
        ok &= s->handleI24StructArray(target.texture_mods);
        ok &= s->handleI24StructArray(target.geometry_mods);
        return ok;
    }
    ok = s->prepare_nested(); // will update the file size left
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
    archive(cereal::make_nvp("SwayRandomize",m.SwayRandomize));
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
