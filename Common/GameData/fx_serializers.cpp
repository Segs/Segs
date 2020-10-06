#include "fx_serializers.h"
#include "fx_definitions.h"
#include "anim_serializers.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <cereal/types/utility.hpp>

namespace {
bool loadFrom(BinStore * s, FxBehavior & target)
{
    s->prepare();
    bool ok = true;
// Unused/Undef fields: "Behavior"

    ok &= s->read(target.name);
    ok &= s->read(target.m_StartJitter);
    ok &= s->read(target.m_InitialVelocity);
    ok &= s->read(target.m_InitialVelocityJitter);
    ok &= s->read(target.m_Gravity);
    ok &= s->read(target.m_Spin);
    ok &= s->read(target.m_SpinJitter);
    ok &= s->read(target.m_FadeInLength);
    ok &= s->read(target.m_FadeOutLength);
    ok &= s->read(target.m_Shake);
    ok &= s->read(target.m_ShakeFallOff);
    ok &= s->read(target.m_ShakeRadius);
    ok &= s->read(target.m_Scale);
    ok &= s->read(target.m_ScaleRate);
    ok &= s->read(target.m_ScaleTime);
    ok &= s->read(target.m_EndScale);
    ok &= s->readU(target.m_Stretch);
    ok &= s->read(target.m_PyrRotate);
    ok &= s->read(target.m_PositionOffset);
    ok &= s->read(target.m_TrackRate);
    ok &= s->read(target.m_TrackMethod);
    ok &= s->readU(target.m_Collides);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_AnimScale);
    ok &= s->readU(target.m_Alpha);
    ok &= s->read(target.m_PulsePeakTime);
    ok &= s->read(target.m_PulseBrightness);
    ok &= s->read(target.m_SplatFlags);
    ok &= s->readEnum(target.m_SplatFalloffType);
    ok &= s->read(target.m_SplatNormalFade);
    ok &= s->read(target.m_SplatFadeCenter);
    ok &= s->read(target.m_SplatSetBack);
    ok &= s->read(target.m_ColorEffect[0].startcolor);
    ok &= s->read(target.m_ColorEffect[1].startcolor);
    ok &= s->read(target.m_ColorEffect[2].startcolor);
    ok &= s->read(target.m_ColorEffect[3].startcolor);
    ok &= s->read(target.m_ColorEffect[4].startcolor);
    ok &= s->read(target.m_ColorEffect[1].startTime);
    ok &= s->read(target.m_ColorEffect[2].startTime);
    ok &= s->read(target.m_ColorEffect[3].startTime);
    ok &= s->read(target.m_ColorEffect[4].startTime);
    ok &= s->read(target.m_ColorEffect[0].startcolor);
    ok &= s->read(target.m_ColorEffect[1].startcolor);
    ok &= s->read(target.m_ColorEffect[2].startcolor);
    ok &= s->read(target.m_ColorEffect[3].startcolor);
    ok &= s->read(target.m_ColorEffect[4].startcolor);
    ok &= s->read(target.m_ColorEffect[1].startTime);
    ok &= s->read(target.m_ColorEffect[2].startTime);
    ok &= s->read(target.m_ColorEffect[3].startTime);
    ok &= s->read(target.m_ColorEffect[4].startTime);
    ok &= s->read(target.m_ColorEffect[0].startTime);

    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("StAnim"==_name) {
            target.m_stAnim.emplace_back();
            ok &= loadFrom(s,target.m_stAnim.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, FxInfo_Input & target)
{
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.m_InpName);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore * s, EventSplat & target)
{
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.tex1);
    ok &= s->read(target.tex2);

    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore * s, FxGeoEntry_Event & target)
{
    s->prepare();
    bool ok = true;

    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.m_EName);
    ok &= s->read(target.m_Type);
    ok &= s->read(target.m_At);
    ok &= s->read(target.m_Bhvr);
    ok &= s->read(target.m_AltPiv);
    ok &= s->read(target.m_AnimPiv);
    ok &= s->read(target.m_Anim);
    ok &= s->read(target.m_SetState);
    ok &= s->read(target.m_ChildFx);
    ok &= s->read(target.m_Magnet);
    ok &= s->read(target.m_LookAt);
    ok &= s->read(target.m_PMagnet);
    ok &= s->read(target.m_POther);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_Power);
    ok &= s->read(target.m_Debris);
    ok &= s->read(target.m_WorldGroup);
    ok &= s->read(target.m_Flags);

    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Geom"==_name) {
            target.m_Geom.emplace_back();
            ok &= s->read(target.m_Geom.back());
        } else if(_name.startsWith("Part")) {
            target.m_Part.emplace_back();
            ok &= s->read(target.m_Part.back());
        } else if(_name.startsWith("Splat")) {
            target.m_Splats.emplace_back();
            ok &= loadFrom(s,target.m_Splats.back());
        } else if(_name.startsWith("Sound")) {
            std::vector<QByteArray> snd_entry;
            ok &= s->read(snd_entry);
            ok &= snd_entry.size()>1;
            FxSoundData true_data;
            true_data.m_Name = snd_entry[0];
            if(snd_entry.size()>1)
                true_data.m_Radius = snd_entry[1].toFloat();
            if(snd_entry.size()>2)
                true_data.m_Fade = snd_entry[2].toFloat();
            if(snd_entry.size()>3)
                true_data.m_Volume = snd_entry[3].toFloat();
            target.m_Sounds.emplace_back(std::move(true_data));
        } else if(_name.startsWith("While")) {
            target.m_While.emplace_back();
            ok &= s->read(target.m_While.back());
        } else if(_name.startsWith("Until")) {
            target.m_Until.emplace_back();
            ok &= s->read(target.m_Until.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore * s, FxInfo_Condition & target)
{
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.m_On);
    ok &= s->read(target.m_Time);
    ok &= s->read(target.m_Dist);
    ok &= s->read(target.m_Chance);
    ok &= s->read(target.DoMany);

    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Event"==_name) {
            target.m_Event.emplace_back();
            ok &= loadFrom(s,target.m_Event.back());
        } else if("TriggerBits"==_name) {
            target.str_TriggerBits.emplace_back();
            ok &= s->read(target.str_TriggerBits.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
bool loadFrom(BinStore * s, FxInfo & target)
{
    int flags=0;
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.fxname);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_Lighting);
    ok &= s->read(flags);
    ok &= s->read(target.m_PerformanceRadius);
    ok &= s->prepare_nested(); // will update the file size left
    target.m_Flags = FxInfo_Flags(flags);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Input"==_name) {
            target.m_Inputs.emplace_back();
            ok &= loadFrom(s,target.m_Inputs.back());
        } else if("Condition"==_name) {
            target.m_Conditions.emplace_back();
            ok &= loadFrom(s,target.m_Conditions.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

}

bool loadFrom(BinStore * s, Fx_AllInfos & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("FxInfo"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
template<class Archive>
static void serialize(Archive & archive, EventSplat & m)
{
    archive(cereal::make_nvp("Tex1",m.tex1));
    archive(cereal::make_nvp("Tex2",m.tex2));
}
template<class Archive>
static void serialize(Archive & archive, FxSoundData & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    // TODO: make the following fields optional
    archive(cereal::make_nvp("Radius",m.m_Radius));
    archive(cereal::make_nvp("Fade",m.m_Fade));
    archive(cereal::make_nvp("Volume",m.m_Volume));
}
template<class Archive>
static void serialize(Archive & archive, FxGeoEntry_Event & m)
{
    archive(cereal::make_nvp("EName",m.m_EName));
    archive(cereal::make_nvp("Type",m.m_Type));
    archive(cereal::make_nvp("At",m.m_At));
    archive(cereal::make_nvp("Bhvr",m.m_Bhvr));
    archive(cereal::make_nvp("AltPiv",m.m_AltPiv));
    archive(cereal::make_nvp("AnimPiv",m.m_AnimPiv));
    archive(cereal::make_nvp("Anim",m.m_Anim));
    archive(cereal::make_nvp("SetState",m.m_SetState));
    archive(cereal::make_nvp("ChildFx",m.m_ChildFx));
    archive(cereal::make_nvp("Magnet",m.m_Magnet));
    archive(cereal::make_nvp("LookAt",m.m_LookAt));
    archive(cereal::make_nvp("PMagnet",m.m_PMagnet));
    archive(cereal::make_nvp("POther",m.m_POther));
    archive(cereal::make_nvp("LifeSpan",m.m_LifeSpan));
    archive(cereal::make_nvp("Power",m.m_Power));
    archive(cereal::make_nvp("Debris",m.m_Debris));
    archive(cereal::make_nvp("WorldGroup",m.m_WorldGroup));
    archive(cereal::make_nvp("Flags",m.m_Flags));
    archive(cereal::make_nvp("Geoms",m.m_Geom));
    archive(cereal::make_nvp("Parts",m.m_Part));
    archive(cereal::make_nvp("Splats",m.m_Splats));
    archive(cereal::make_nvp("Sounds",m.m_Sounds));
    archive(cereal::make_nvp("Whiles",m.m_While));
    archive(cereal::make_nvp("Untils",m.m_Until));
}

template<class Archive>
static void serialize(Archive & archive, FxInfo_Condition & m)
{
    archive(cereal::make_nvp("On",m.m_On));
    archive(cereal::make_nvp("Time",m.m_Time));
    archive(cereal::make_nvp("Dist",m.m_Dist));
    archive(cereal::make_nvp("Chance",m.m_Chance));
    archive(cereal::make_nvp("DoMany",m.DoMany));
    archive(cereal::make_nvp("Events",m.m_Event));
    archive(cereal::make_nvp("TriggerBits",m.str_TriggerBits));
}
template<class Archive>
static void serialize(Archive & archive, FxInfo & m)
{
    archive(cereal::make_nvp("fxname",m.fxname));
    archive(cereal::make_nvp("LifeSpan",m.m_LifeSpan));
    archive(cereal::make_nvp("Lighting",m.m_Lighting));
    archive(cereal::make_nvp("Flags",m.m_Flags));
    archive(cereal::make_nvp("PerformanceRadius",m.m_PerformanceRadius));
    archive(cereal::make_nvp("Inputs",m.m_Inputs));
    archive(cereal::make_nvp("Conditions",m.m_Conditions));
}
template<class Archive>
static void serialize(Archive & archive, FxInfo_Input & m)
{
    archive(cereal::make_nvp("InpName",m.m_InpName));
}
void saveTo(const Fx_AllInfos &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"AllFxInfos_Data",baseName,text_format);
}
bool loadFrom(const QString &filepath, Fx_AllInfos &target)
{
    QFSWrapper wrap;
    return commonReadFrom(wrap,filepath,"AllFxInfos_Data",target);
}
bool LoadFxInfoData(const QString &fname, Fx_AllInfos &infos)
{
    QFSWrapper wrap;
    BinStore binfile;

    if(fname.contains(".crl"))
    {
        if(!loadFrom(fname, infos))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    if(!binfile.open(wrap,fname, fxbehaviors_i0_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if(!loadFrom(&binfile, infos))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
}


bool loadFrom(BinStore * s, Fx_AllBehaviors & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Behavior"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
template<class Archive>
void serialize(Archive & archive, ColorFx & m)
{
    archive(cereal::make_nvp("StartColor",m.startcolor));
    archive(cereal::make_nvp("StartTime",m.startTime));
}

template<class Archive>
static void serialize(Archive & archive, FxBehavior & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("StartJitter",m.m_StartJitter));
    archive(cereal::make_nvp("InitialVelocity",m.m_InitialVelocity));
    archive(cereal::make_nvp("InitialVelocityJitter",m.m_InitialVelocityJitter));
    archive(cereal::make_nvp("Gravity",m.m_Gravity));
    archive(cereal::make_nvp("Spin",m.m_Spin));
    archive(cereal::make_nvp("SpinJitter",m.m_SpinJitter));
    archive(cereal::make_nvp("FadeInLength",m.m_FadeInLength));
    archive(cereal::make_nvp("FadeOutLength",m.m_FadeOutLength));
    archive(cereal::make_nvp("Shake",m.m_Shake));
    archive(cereal::make_nvp("ShakeFallOff",m.m_ShakeFallOff));
    archive(cereal::make_nvp("ShakeRadius",m.m_ShakeRadius));
    archive(cereal::make_nvp("Scale",m.m_Scale));
    archive(cereal::make_nvp("ScaleRate",m.m_ScaleRate));
    archive(cereal::make_nvp("ScaleTime",m.m_ScaleTime));
    archive(cereal::make_nvp("EndScale",m.m_EndScale));
    archive(cereal::make_nvp("Stretch",m.m_Stretch));
    archive(cereal::make_nvp("PyrRotate",m.m_PyrRotate));
    archive(cereal::make_nvp("PositionOffset",m.m_PositionOffset));
    archive(cereal::make_nvp("TrackRate",m.m_TrackRate));
    archive(cereal::make_nvp("TrackMethod",m.m_TrackMethod));
    archive(cereal::make_nvp("Collides",m.m_Collides));
    archive(cereal::make_nvp("LifeSpan",m.m_LifeSpan));
    archive(cereal::make_nvp("AnimScale",m.m_AnimScale));
    archive(cereal::make_nvp("Alpha",m.m_Alpha));
    archive(cereal::make_nvp("PulsePeakTime",m.m_PulsePeakTime));
    archive(cereal::make_nvp("PulseBrightness",m.m_PulseBrightness));
    archive(cereal::make_nvp("SplatFlags",m.m_SplatFlags));
    archive(cereal::make_nvp("SplatFalloffType",m.m_SplatFalloffType));
    archive(cereal::make_nvp("SplatNormalFade",m.m_SplatNormalFade));
    archive(cereal::make_nvp("SplatFadeCenter",m.m_SplatFadeCenter));
    archive(cereal::make_nvp("SplatSetBack",m.m_SplatSetBack));
    archive(cereal::make_nvp("ColorEffects",m.m_ColorEffect));

    archive(cereal::make_nvp("StAnim",m.m_stAnim));
}

void saveTo(const Fx_AllBehaviors &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"Fx_AllBehaviors",baseName,text_format);
}
bool loadFrom(const QString &filepath, Fx_AllBehaviors &target)
{
    QFSWrapper wrap;
    return commonReadFrom(wrap,filepath,"Fx_AllBehaviors",target);
}
bool LoadFxBehaviorData(const QString &fname, Fx_AllBehaviors &behaviors)
{
    QFSWrapper wrap;
    BinStore binfile;

    if(fname.contains(".crl"))
    {
        if(!loadFrom(fname, behaviors))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    if(!binfile.open(wrap,fname, fxbehaviors_i0_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if(!loadFrom(&binfile, behaviors))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
}
