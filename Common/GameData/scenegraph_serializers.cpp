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

#include "scenegraph_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "DataStorage.h"
#include "scenegraph_definitions.h"
#include "Components/Logging.h"

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
        ok &= s->read(target.Size);
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
        ok &= s->read(target.name);
        ok &= s->read(target.amplitude);
        ok &= s->prepare_nested();
        assert(ok);
        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, DefFog_Data &target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target.fogZ);
        ok &= s->read(target.fogX);
        ok &= s->read(target.fogY);
        ok &= s->read(target.fogClr1);
        ok &= s->read(target.fogClr2);
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
        ok &= s->read(target.name);
        ok &= s->read(target.type);
        ok &= s->read(target.flags);
        ok &= s->read(target.p_Obj);
        ok &= s->prepare_nested(); // will update the file size left
        assert(ok);
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("Group"==_name) {
                target.p_Grp.emplace_back();
                ok &= loadFrom(s,target.p_Grp.back());
            } else if("Property"==_name) {
                target.p_Property.emplace_back();
                ok &= loadFrom(s,target.p_Property.back());
            } else if("TintColor"==_name) {
                target.p_TintColor.emplace_back();
                ok &= loadFrom(s,target.p_TintColor.back());
            } else if("Ambient"==_name) {
                target.p_Ambient.emplace_back();
                ok &= loadFrom(s,target.p_Ambient.back());
            } else if("Omni"==_name) {
                target.p_Omni.emplace_back();
                ok &= loadFrom(s,target.p_Omni.back());
            } else if("Sound"==_name) {
                target.p_Sound.emplace_back();
                ok &= loadFrom(s,target.p_Sound.back());
            } else if("ReplaceTex"==_name) {
                target.p_ReplaceTex.emplace_back();
                ok &= loadFrom(s,target.p_ReplaceTex.back());
            } else if("Beacon"==_name) {
                target.p_Beacon.emplace_back();
                ok &= loadFrom(s,target.p_Beacon.back());
            } else if("Fog"==_name) {
                target.p_Fog.emplace_back();
                ok &= loadFrom(s,target.p_Fog.back());
            } else if("Lod"==_name) {
                target.p_Lod.emplace_back();
                ok &= loadFrom(s,target.p_Lod.back());
            } else
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

bool loadFrom(BinStore *s, SceneGraph_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.Version);
    ok &= s->read(target.Scenefile);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Def"==_name || "RootMod"==_name) {
            target.Def.emplace_back();
            ok &= loadFrom(s,target.Def.back());
        } else if("Ref"==_name) {
            target.Ref.emplace_back();
            ok &= loadFrom(s,target.Ref.back());
        } else

            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, TintColor_Data & m)
{
    archive(cereal::make_nvp("Color1",m.clr1));
    archive(cereal::make_nvp("Color2",m.clr2));
}

template<class Archive>
static void serialize(Archive & archive, ReplaceTex_Data & m)
{
    archive(cereal::make_nvp("TextureID",m.texIdxToReplace));
    archive(cereal::make_nvp("NewTexture",m.repl_with));
}

template<class Archive>
static void serialize(Archive & archive, DefOmni_Data & m)
{
    archive(cereal::make_nvp("Color",m.omniColor));
    archive(cereal::make_nvp("Size",m.Size));
    archive(cereal::make_nvp("Flags",m.isNegative));
}

template<class Archive>
static void serialize(Archive & archive, DefBeacon_Data & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("Amplitude",m.amplitude));
}

template<class Archive>
static void serialize(Archive & archive, DefFog_Data & m)
{
    archive(cereal::make_nvp("Z",m.fogZ));
    archive(cereal::make_nvp("X",m.fogX));
    archive(cereal::make_nvp("Y",m.fogY));
    archive(cereal::make_nvp("Color1",m.fogClr1));
    archive(cereal::make_nvp("Color2",m.fogClr2));
}

template<class Archive>
static void serialize(Archive & archive, DefAmbient_Data & m)
{
    archive(cereal::make_nvp("Color",m.clr));
}

template<class Archive>
static void serialize(Archive & archive, DefLod_Data & m)
{
    archive(cereal::make_nvp("Far",m.Far));
    archive(cereal::make_nvp("FarFade",m.FarFade));
    archive(cereal::make_nvp("Near",m.Near));
    archive(cereal::make_nvp("NearFade",m.NearFade));
    archive(cereal::make_nvp("Scale",m.Scale));
}

template<class Archive>
static void serialize(Archive & archive, DefSound_Data & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("Volume",m.volRel1));
    archive(cereal::make_nvp("Radius",m.sndRadius));
    archive(cereal::make_nvp("RampInFeet",m.snd_ramp_feet));
    archive(cereal::make_nvp("Flags",m.sndFlags));
}

template<class Archive>
static void serialize(Archive & archive, GroupProperty_Data & m)
{
    archive(cereal::make_nvp("Name",m.propName));
    archive(cereal::make_nvp("Value",m.propValue));
    archive(cereal::make_nvp("Type",m.propertyType));
}

template<class Archive>
static void serialize(Archive & archive, GroupLoc_Data & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("Position",m.pos));
    archive(cereal::make_nvp("Rotation",m.rot));
}

template<class Archive>
static void serialize(Archive & archive, SceneGraphNode_Data & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("Object",m.p_Obj));
    archive(cereal::make_nvp("Type",m.type));
    archive(cereal::make_nvp("Flags",m.flags));

    archive(cereal::make_nvp("Groups",m.p_Grp));
    archive(cereal::make_nvp("Properties",m.p_Property));
    archive(cereal::make_nvp("Tints",m.p_TintColor));
    archive(cereal::make_nvp("Sounds",m.p_Sound));
    archive(cereal::make_nvp("TextureReplacements",m.p_ReplaceTex));
    archive(cereal::make_nvp("Omnis",m.p_Omni));
    archive(cereal::make_nvp("Beacons",m.p_Beacon));
    archive(cereal::make_nvp("Fogs",m.p_Fog));
    archive(cereal::make_nvp("AmbientColors",m.p_Ambient));
    archive(cereal::make_nvp("Lods",m.p_Lod));
}

template<class Archive>
static void serialize(Archive & archive, SceneRootNode_Data & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("Position",m.pos));
    archive(cereal::make_nvp("Rotation",m.rot));
}

template<class Archive>
static void serialize(Archive & archive, SceneGraph_Data & m)
{
    archive(cereal::make_nvp("Version",m.Version));
    archive(cereal::make_nvp("Scenefile",m.Scenefile));
    archive(cereal::make_nvp("Defs",m.Def));
    archive(cereal::make_nvp("Refs",m.Ref));
}

void saveTo(const SceneGraph_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"SceneGraph",baseName,text_format);
}

bool loadFrom(FSWrapper &fs, const QString &filepath, SceneGraph_Data &target)
{
    return commonReadFrom(fs,filepath,"SceneGraph",target);
}
QString getFilepathCaseInsensitive(FSWrapper& fs, QString fpath)
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
    for (QString& f : files)
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
    QString fixed_path = getFilepathCaseInsensitive(fs,fname);

    if(fixed_path.contains(".crl"))
    {
        if(!loadFrom(fs, fixed_path, scenegraph))
        {
            qCritical() << "Failed to serialize data from crl:" << fixed_path;
            return false;
        }
        return true;
    }
    if(!binfile.open(fs,fixed_path, scenegraph_i0_2_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fixed_path;
        return false;
    }
    if(!loadFrom(&binfile, scenegraph))
    {
        qCritical() << "Failed to load data from original bin:" << fixed_path;
        return false;
    }
    return true;
}
//! @}
