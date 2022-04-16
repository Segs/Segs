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

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "scenegraph_serializers.h"

#include "Components/Logging.h"
#include "DataStorage.h"
#include "scenegraph_definitions.h"

#include <QFileInfo>

namespace
{
bool loadFrom(BinStore *s, GroupProperty_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.propName);
    ok &= s->read(target.propValue);
    ok &= s->read(target.propertyType);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, TintColor_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.clr1);
    ok &= s->read(target.clr2);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefAmbient_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.clr);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefOmni_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.omniColor);
    ok &= s->read(target.Radius);
    ok &= s->read(target.isNegative);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefSound_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.volRel1);
    ok &= s->read(target.sndRadius);
    ok &= s->read(target.snd_ramp_feet);
    ok &= s->read(target.sndFlags);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, ReplaceTex_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.texIdxToReplace);
    ok &= s->read(target.repl_with);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefBeacon_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_Radius);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefFog_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Radius);
    ok &= s->read(target.m_NearDistance);
    ok &= s->read(target.m_FarDistance);
    ok &= s->read(target.Color1);
    ok &= s->read(target.Color2);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, DefLod_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.Far);
    ok &= s->read(target.FarFade);
    ok &= s->read(target.Near);
    ok &= s->read(target.NearFade);
    ok &= s->read(target.Scale);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, GroupLoc_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.pos);
    ok &= s->read(target.rot);
    ok &= s->prepare_nested();
    assert(ok);
    assert(s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, SceneGraphNode_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_Type);
    ok &= s->readEnum(target.m_Flags);
    ok &= s->read(target.m_Object);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Group" == _name)
        {
            target.m_Group.emplace_back();
            ok &= loadFrom(s, target.m_Group.back());
        }
        else if ("Property" == _name)
        {
            target.m_Property.emplace_back();
            ok &= loadFrom(s, target.m_Property.back());
        }
        else if ("TintColor" == _name)
        {
            target.m_TintColor.emplace_back();
            ok &= loadFrom(s, target.m_TintColor.back());
        }
        else if ("Ambient" == _name)
        {
            target.m_Ambient.emplace_back();
            ok &= loadFrom(s, target.m_Ambient.back());
        }
        else if ("Omni" == _name)
        {
            target.m_Omni.emplace_back();
            ok &= loadFrom(s, target.m_Omni.back());
        }
        else if ("Sound" == _name)
        {
            target.m_Sound.emplace_back();
            ok &= loadFrom(s, target.m_Sound.back());
        }
        else if ("ReplaceTex" == _name)
        {
            target.m_ReplaceTex.emplace_back();
            ok &= loadFrom(s, target.m_ReplaceTex.back());
        }
        else if ("Beacon" == _name)
        {
            target.m_Beacon.emplace_back();
            ok &= loadFrom(s, target.m_Beacon.back());
        }
        else if ("Fog" == _name)
        {
            target.m_Fog.emplace_back();
            ok &= loadFrom(s, target.m_Fog.back());
        }
        else if ("Lod" == _name)
        {
            target.m_Lod.emplace_back();
            ok &= loadFrom(s, target.m_Lod.back());
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, SceneRootNode_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.pos);
    ok &= s->read(target.rot);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    assert(s->end_encountered());
    return ok;
}
} // namespace
static bool loadFromI24(BinStore *s, TexSwap_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_TextureName);
    ok &= s->read(target.m_ReplaceWith);
    ok &= s->read(target.m_Composite);
    return ok;
}

static bool loadFromI24(BinStore *s, DefLod_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.Far);
    ok &= s->read(target.FarFade);
    ok &= s->read(target.Scale);
    return ok;
}

static bool loadFromI24(BinStore *s, DefFog_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Radius);
    ok &= s->read(target.m_NearDistance);
    ok &= s->read(target.m_FarDistance);
    ok &= s->read(target.Color1);
    ok &= s->read(target.Color2);
    ok &= s->read(target.m_Speed);
    return ok;
}
static bool loadFromI24(BinStore *s, DefBeacon_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_Radius);
    ok &= s->prepare_nested();
    return ok;
}

static bool loadFromI24(BinStore *s, ReplaceTex_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.texIdxToReplace);
    ok &= s->read(target.repl_with);
    return ok;
}

static bool loadFromI24(BinStore *s, DefSound_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.volRel1);
    ok &= s->read(target.sndRadius);
    ok &= s->read(target.snd_ramp_feet);
    ok &= s->read(target.sndFlags);
    return ok;
}

static bool loadFromI24(BinStore *s, Volume_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Scale);
    return ok;
}

static bool loadFromI24(BinStore *s, Cubemap_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_GenerateSize);
    ok &= s->read(target.m_CaptureSize);
    ok &= s->read(target.m_Blur);
    ok &= s->read(target.m_CaptureSize);
    return ok;
}

static bool loadFromI24(BinStore *s, DefOmni_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.omniColor);
    ok &= s->read(target.Radius);
    ok &= s->read(target.isNegative);
    return ok;
}

static bool loadFromI24(BinStore *s, DefAmbient_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.clr);
    return ok;
}
static bool loadFromI24(BinStore *s, TintColor_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.clr1);
    ok &= s->read(target.clr2);
    return ok;
}
static bool loadFromI24(BinStore *s, GroupProperty_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.propName);
    ok &= s->read(target.propValue);
    ok &= s->read(target.propertyType);
    return ok;
}
static bool loadFromI24(BinStore *s, GroupLoc_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.pos);
    ok &= s->read(target.rot);
    ok &= s->readEnum(target.flags);
    return ok;
}

static bool loadFromI24(BinStore *s, SceneGraphNode_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->handleI24StructArray(target.m_Group);
    ok &= s->handleI24StructArray(target.m_Property);
    ok &= s->handleI24StructArray(target.m_TintColor);
    ok &= s->handleI24StructArray(target.m_Ambient);
    ok &= s->handleI24StructArray(target.m_Omni);
    ok &= s->handleI24StructArray(target.m_Cubemap);
    ok &= s->handleI24StructArray(target.m_Volume);
    ok &= s->handleI24StructArray(target.m_Sound);
    ok &= s->handleI24StructArray(target.m_ReplaceTex);
    ok &= s->handleI24StructArray(target.m_Beacon);
    ok &= s->handleI24StructArray(target.m_Fog);
    ok &= s->handleI24StructArray(target.m_Lod);
    ok &= s->read(target.m_Type);
    ok &= s->readEnum(target.m_Flags);
    ok &= s->read(target.m_Alpha);
    ok &= s->read(target.m_Object);
    ok &= s->handleI24StructArray(target.m_TexSwap);
    ok &= s->read(target.m_SoundScript);
    return ok;
}
static bool loadFromI24(BinStore *s, SceneRootNode_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.pos);
    ok &= s->read(target.rot);
    return ok;
}

bool loadFrom(BinStore *s, SceneGraph_Data &target)
{
    bool ok = true;
    s->prepare();
    if (s->isI24Data())
    {

        ok &= s->read(target.Version);
        ok &= s->read(target.Scenefile);
        ok &= s->read(target.LoadScreen);
        ok &= s->handleI24StructArray(target.Def);
        ok &= s->handleI24StructArray(target.Ref);
        ok &= s->read(target.Import);
        return ok;
    }
    ok &= s->read(target.Version);
    ok &= s->read(target.Scenefile);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Def" == _name || "RootMod" == _name)
        {
            target.Def.emplace_back();
            ok &= loadFrom(s, target.Def.back());
        }
        else if ("Ref" == _name)
        {
            target.Ref.emplace_back();
            ok &= loadFrom(s, target.Ref.back());
        }
        else

            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

template <class Archive> static void serialize(Archive &archive, TintColor_Data &m)
{
    archive(cereal::make_nvp("Color1", m.clr1));
    archive(cereal::make_nvp("Color2", m.clr2));
}

template <class Archive> static void serialize(Archive &archive, ReplaceTex_Data &m)
{
    archive(cereal::make_nvp("TextureID", m.texIdxToReplace));
    archive(cereal::make_nvp("NewTexture", m.repl_with));
}

template <class Archive> static void serialize(Archive &archive, DefOmni_Data &m)
{
    archive(cereal::make_nvp("Color", m.omniColor));
    archive(cereal::make_nvp("Radius", m.Radius));
    archive(cereal::make_nvp("Flags", m.isNegative));
}

template <class Archive> static void serialize(Archive &archive, Cubemap_Data &m)
{
    archive(cereal::make_nvp("GenerateSize", m.m_GenerateSize));
    archive(cereal::make_nvp("CaptureSize", m.m_CaptureSize));
    archive(cereal::make_nvp("Blur", m.m_Blur));
    archive(cereal::make_nvp("Time", m.m_Time));
}

template <class Archive> static void serialize(Archive &archive, Volume_Data &m)
{
    archive(cereal::make_nvp("Scale", m.m_Scale));
}

template <class Archive> static void serialize(Archive &archive, TexSwap_Data &m)
{
    archive(cereal::make_nvp("TextureName", m.m_TextureName));
    archive(cereal::make_nvp("ReplaceWith", m.m_ReplaceWith));
    archive(cereal::make_nvp("Composite", m.m_Composite));
}

template <class Archive> static void serialize(Archive &archive, DefBeacon_Data &m)
{
    archive(cereal::make_nvp("Name", m.m_Name));
    archive(cereal::make_nvp("Amplitude", m.m_Radius));
}

template <class Archive> static void serialize(Archive &archive, DefFog_Data &m)
{
    archive(cereal::make_nvp("Radius", m.m_Radius));
    archive(cereal::make_nvp("NearDistance", m.m_NearDistance));
    archive(cereal::make_nvp("FarDistance", m.m_FarDistance));
    archive(cereal::make_nvp("Color1", m.Color1));
    archive(cereal::make_nvp("Color2", m.Color2));
    archive(cereal::make_nvp("Speed", m.m_Speed));
}

template <class Archive> static void serialize(Archive &archive, DefAmbient_Data &m)
{
    archive(cereal::make_nvp("Color", m.clr));
}

template <class Archive> static void serialize(Archive &archive, DefLod_Data &m)
{
    archive(cereal::make_nvp("Far", m.Far));
    archive(cereal::make_nvp("FarFade", m.FarFade));
    archive(cereal::make_nvp("Near", m.Near));
    archive(cereal::make_nvp("NearFade", m.NearFade));
    archive(cereal::make_nvp("Scale", m.Scale));
}

template <class Archive> static void serialize(Archive &archive, DefSound_Data &m)
{
    archive(cereal::make_nvp("Name", m.name));
    archive(cereal::make_nvp("Volume", m.volRel1));
    archive(cereal::make_nvp("Radius", m.sndRadius));
    archive(cereal::make_nvp("RampInFeet", m.snd_ramp_feet));
    archive(cereal::make_nvp("Flags", m.sndFlags));
}

template <class Archive> static void serialize(Archive &archive, GroupProperty_Data &m)
{
    archive(cereal::make_nvp("Name", m.propName));
    archive(cereal::make_nvp("Value", m.propValue));
    archive(cereal::make_nvp("Type", m.propertyType));
}

template <class Archive> static void serialize(Archive &archive, GroupLoc_Data &m)
{
    archive(cereal::make_nvp("Name", m.name));
    archive(cereal::make_nvp("Position", m.pos));
    archive(cereal::make_nvp("Rotation", m.rot));
    archive(cereal::make_nvp("Flags", m.flags));
}

template <class Archive> static void serialize(Archive &archive, SceneGraphNode_Data &m)
{
    archive(cereal::make_nvp("Name", m.m_Name));
    archive(cereal::make_nvp("Object", m.m_Object));
    archive(cereal::make_nvp("Type", m.m_Type));
    archive(cereal::make_nvp("SoundScript", m.m_SoundScript));
    archive(cereal::make_nvp("Group", m.m_Group));
    archive(cereal::make_nvp("Property", m.m_Property));
    archive(cereal::make_nvp("TintColor", m.m_TintColor));
    archive(cereal::make_nvp("Sounds", m.m_Sound));
    archive(cereal::make_nvp("ReplaceTextures", m.m_ReplaceTex));
    archive(cereal::make_nvp("Omnis", m.m_Omni));
    archive(cereal::make_nvp("Cubemaps", m.m_Cubemap));
    archive(cereal::make_nvp("Volumes", m.m_Volume));
    archive(cereal::make_nvp("Beacons", m.m_Beacon));
    archive(cereal::make_nvp("Fogs", m.m_Fog));
    archive(cereal::make_nvp("Ambients", m.m_Ambient));
    archive(cereal::make_nvp("Lods", m.m_Lod));
    archive(cereal::make_nvp("TexSwaps", m.m_TexSwap));
    archive(cereal::make_nvp("Flags", m.m_Flags));
    archive(cereal::make_nvp("Alpha", m.m_Alpha));
}

template <class Archive> static void serialize(Archive &archive, SceneRootNode_Data &m)
{
    archive(cereal::make_nvp("Name", m.name));
    archive(cereal::make_nvp("Position", m.pos));
    archive(cereal::make_nvp("Rotation", m.rot));
}

template <class Archive> static void serialize(Archive &archive, SceneGraph_Data &m)
{
    archive(cereal::make_nvp("Version", m.Version));
    archive(cereal::make_nvp("Scenefile", m.Scenefile));
    archive(cereal::make_nvp("Defs", m.Def));
    archive(cereal::make_nvp("Refs", m.Ref));
}

void saveTo(const SceneGraph_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target, "SceneGraph", baseName, text_format);
}

bool loadFrom(FSWrapper &fs, const QString &filepath, SceneGraph_Data &target)
{
    return commonReadFrom(fs, filepath, "SceneGraph", target);
}
QString getFilepathCaseInsensitive(FSWrapper &fs, QString fpath)
{
    // Windows is far too lax about case sensitivity. Consequently
    // filenames aren't consistent. This should derive the filename
    // based upon a case-insensitive comparison, and use the actual
    // formatted filepath when loading scene data.

    // check file exists, if so, return original path
    if (fs.exists(fpath))
        return fpath;

    // get base from path
    QString base_path = QFileInfo(fpath).path();

    if (!fs.exists(base_path))
        qWarning() << "Failed to open" << base_path;

    QStringList files = fs.dir_entries(base_path);
    for (QString &f : files)
    {
        qCDebug(logSceneGraph) << "Comparing" << f << fpath;
        if (fpath.endsWith(f, Qt::CaseInsensitive))
            fpath = base_path + "/" + f;
    }

    return fpath;
}
bool LoadSceneData(FSWrapper &fs, const QString &fname, SceneGraph_Data &scenegraph)
{
    BinStore binfile;
    QString  fixed_path = getFilepathCaseInsensitive(fs, fname);

    if (fixed_path.contains(".crl"))
    {
        if (!loadFrom(fs, fixed_path, scenegraph))
        {
            qCritical() << "Failed to serialize data from crl:" << fixed_path;
            return false;
        }
        return true;
    }
    if (!binfile.open(fs, fixed_path, scenegraph_i0_2_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fixed_path;
        return false;
    }
    if (!loadFrom(&binfile, scenegraph))
    {
        qCritical() << "Failed to load data from original bin:" << fixed_path;
        return false;
    }
    return true;
}
//! @}
