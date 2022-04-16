#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "DataStorage.h"
#include "anim_serializers.h"
#include "fx_definitions.h"
#include "fx_serializers.h"
#include <cereal/types/utility.hpp>

namespace
{
bool loadFrom(BinStore *s, FxBehavior &target)
{
    s->prepare();
    bool ok = true;
    // Unused/Undef fields: "Behavior"

    ok &= s->read(target.m_Name);
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
    ok &= s->readEnum(target.m_SplatFlags);
    ok &= s->readEnum(target.m_SplatFalloffType);
    ok &= s->read(target.m_SplatNormalFade);
    ok &= s->read(target.m_SplatFadeCenter);
    ok &= s->read(target.m_SplatSetBack);
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startcolor); //
    target.m_ColorEffect[0].startTime = 0;
    for (int i = 1; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startTime);
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startcolor); //
    ok &= s->read(target.m_ColorEffect[1].startTime);
    ok &= s->read(target.m_ColorEffect[2].startTime);
    ok &= s->read(target.m_ColorEffect[3].startTime);
    ok &= s->read(target.m_ColorEffect[4].startTime);
    ok &= s->read(target.m_ColorEffect[0].startTime);

    ok &= s->prepare_nested(); // will update the file size left
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("StAnim" == _name)
        {
            target.m_StAnim.emplace_back();
            ok &= loadFrom(s, target.m_StAnim.back());
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore *s, FxInfo_Input &target)
{
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.m_InpName);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, EventSplat &target)
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

bool loadFrom(BinStore *s, FxGeoEntry_Event &target)
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
    ok &= s->readEnum(target.m_Flags);

    ok &= s->prepare_nested(); // will update the file size left
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Geom" == _name)
        {
            assert(target.m_Geom.empty());
            ok &= s->read(target.m_Geom);
        }
        else if (_name.startsWith("Part"))
        {
            assert(target.m_Part.empty());
            ok &= s->read(target.m_Part);
        }
        else if (_name.startsWith("Splat"))
        {
            target.m_Splats.emplace_back();
            ok &= loadFrom(s, target.m_Splats.back());
        }
        else if (_name.startsWith("Sound"))
        {
            std::vector<QByteArray> snd_entry;
            ok &= s->read(snd_entry);
            ok &= snd_entry.size() > 1;
            FxSoundData true_data;
            true_data.m_Name = snd_entry[0];
            if (snd_entry.size() > 1)
                true_data.m_Radius = snd_entry[1].toFloat();
            if (snd_entry.size() > 2)
                true_data.m_Fade = snd_entry[2].toFloat();
            if (snd_entry.size() > 3)
                true_data.m_Volume = snd_entry[3].toFloat();
            target.m_Sounds.emplace_back(std::move(true_data));
        }
        else if (_name.startsWith("While"))
        {
            assert(target.m_While.empty());
            ok &= s->read(target.m_While);
        }
        else if (_name.startsWith("Until"))
        {
            assert(target.m_Until.empty());
            ok &= s->read(target.m_Until);
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

bool loadFrom(BinStore *s, FxInfo_Condition &target)
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
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Event" == _name)
        {
            target.m_Event.emplace_back();
            ok &= loadFrom(s, target.m_Event.back());
        }
        else if ("TriggerBits" == _name)
        {
            assert(target.str_TriggerBits.empty());
            ok &= s->read(target.str_TriggerBits);
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
bool loadFrom(BinStore *s, FxInfo &target)
{
    int flags = 0;
    s->prepare();
    bool ok = true;
    // Unused/Undef tokens: FxInfo
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_Lighting);
    ok &= s->read(flags);
    ok &= s->read(target.m_PerformanceRadius);
    ok &= s->prepare_nested(); // will update the file size left
    target.m_Flags = FxInfo_Flags(flags);
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Input" == _name)
        {
            target.m_Inputs.emplace_back();
            ok &= loadFrom(s, target.m_Inputs.back());
        }
        else if ("Condition" == _name)
        {
            target.m_Conditions.emplace_back();
            ok &= loadFrom(s, target.m_Conditions.back());
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

} // namespace

static bool loadFromI24(BinStore *s, FxBehavior &target)
{
    s->prepare();
    bool     ok = true;
    uint32_t size;
    ok &= s->read(size);
    if (ok)
        target.m_ParamBitfield.resize(size);
    ok &= s->read_bytes((char *)target.m_ParamBitfield.data(), size);
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileAge);
    ok &= s->read(target.m_StartJitter);
    ok &= s->read(target.m_InitialVelocity);
    ok &= s->read(target.m_InitialVelocityJitter);
    ok &= s->read(target.m_InitialVelocityAngle);
    ok &= s->read(target.m_Gravity);
    ok &= s->read(target.m_Physics);
    ok &= s->read(target.m_PhysRadius);
    ok &= s->read(target.m_PhysGravity);
    ok &= s->read(target.m_PhysRestitution);
    ok &= s->read(target.m_PhysSFriction);
    ok &= s->read(target.m_PhysDFriction);
    ok &= s->read(target.m_PhysKillBelowSpeed);
    ok &= s->read(target.m_PhysDensity);
    ok &= s->read(target.m_PhysForceRadius);
    ok &= s->read(target.m_PhysForcePower);
    ok &= s->read(target.m_PhysForcePowerJitter);
    ok &= s->read(target.m_PhysForceCentripetal);
    ok &= s->read(target.m_PhysForceSpeedScaleMax);
    ok &= s->read(target.m_PhysScale);

    ok &= s->readEnum(target.m_PhysJointDOFs);
    ok &= s->read(target.m_PhysJointAnchor);
    ok &= s->read(target.m_PhysJointAngLimit);
    ok &= s->read(target.m_PhysJointLinLimit);
    ok &= s->read(target.m_PhysJointBreakTorque);
    ok &= s->read(target.m_PhysJointBreakForce);
    ok &= s->read(target.m_PhysJointLinSpring);
    ok &= s->read(target.m_PhysJointLinSpringDamp);
    ok &= s->read(target.m_PhysJointAngSpring);
    ok &= s->read(target.m_PhysJointAngSpringDamp);
    ok &= s->read(target.m_PhysJointDrag);
    ok &= s->read(target.m_PhysJointCollidesWorld);
    ok &= s->readEnum(target.m_PhysForceType);
    ok &= s->readEnum(target.m_PhysDebris);

    ok &= s->read(target.m_Spin);
    ok &= s->read(target.m_SpinJitter);
    ok &= s->read(target.m_FadeInLength);
    ok &= s->read(target.m_FadeOutLength);
    ok &= s->read(target.m_Shake);
    ok &= s->read(target.m_ShakeFallOff);
    ok &= s->read(target.m_ShakeRadius);
    ok &= s->read(target.m_Blur);
    ok &= s->read(target.m_BlurFallOff);
    ok &= s->read(target.m_BlurRadius);
    ok &= s->read(target.m_Scale);
    ok &= s->read(target.m_ScaleRate);
    ok &= s->read(target.m_ScaleTime);
    ok &= s->read(target.m_EndScale);
    ok &= s->read(target.m_Stretch);
    ok &= s->read(target.m_Drag);
    ok &= s->read(target.m_PyrRotate);
    ok &= s->read(target.m_PyrRotateJitter);
    ok &= s->read(target.m_PositionOffset);
    ok &= s->read(target.m_UseShieldOffset);
    ok &= s->read(target.m_TrackRate);
    ok &= s->read(target.m_TrackMethod);
    ok &= s->read(target.m_Collides);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_AnimScale);
    ok &= s->read(target.m_Alpha);
    ok &= s->read(target.m_PulsePeakTime);
    ok &= s->read(target.m_PulseBrightness);
    ok &= s->read(target.m_PulseClamp);
    ok &= s->readEnum(target.m_SplatFlags);
    ok &= s->readEnum(target.m_SplatFalloffType);
    ok &= s->read(target.m_SplatNormalFade);
    ok &= s->read(target.m_SplatFadeCenter);
    ok &= s->read(target.m_SplatSetBack);
    ok &= s->handleI24StructArray(target.m_StAnim);
    ok &= s->read(target.m_HueShift);
    ok &= s->read(target.m_HueShiftJitter);
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startcolor); //
    target.m_ColorEffect[0].startTime = 0;
    for (int i = 1; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startTime);
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].primaryTint);
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].secondaryTint);

    ok &= s->read(target.m_InheritGroupTint);

    // duplicated data ??
    for (int i = 0; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startcolor);
    for (int i = 1; i < 5; ++i)
        ok &= s->read(target.m_ColorEffect[i].startTime);
    ok &= s->read(target.m_ColorEffect[0].startTime);
    ok &= s->read(target.m_TintGeom);
    return ok;
}

static bool loadFromI24(BinStore *s, FxSoundData &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_Radius);
    ok &= s->read(target.m_Fade);
    ok &= s->read(target.m_Volume);
    return ok;
}

static bool loadFromI24(BinStore *s, EventSplat &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.tex1);
    ok &= s->read(target.tex2);
    return ok;
}

static bool loadFromI24(BinStore *s, FxGeoEntry_Event &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_EName);
    ok &= s->read(target.m_Type);
    ok &= s->readEnum(target.m_Inherit); // FxTransformFlag
    ok &= s->readEnum(target.m_Update);  // FxTransformFlag
    ok &= s->read(target.m_At);
    ok &= s->read(target.m_Bhvr);
    ok &= s->handleI24StructArray(target.m_BhvrOverride);
    ok &= s->read(target.m_JEvent);
    ok &= s->read(target.m_CEvent);
    ok &= s->read(target.m_CDestroy);
    ok &= s->read(target.m_JDestroy);
    ok &= s->readEnum(target.m_CRotation); // enum FxCeventCollisionRotation
    ok &= s->read(target.m_ParentVelocityFraction);
    ok &= s->read(target.m_CThresh);
    ok &= s->read(target.m_HardwareOnly); // bool value
    ok &= s->read(target.m_SoftwareOnly); // bool value
    ok &= s->read(target.m_PhysicsOnly);  // bool value
    ok &= s->read(target.m_CameraSpace);  // bool value
    ok &= s->read(target.m_RayLength);
    ok &= s->read(target.m_AtRayFx);
    ok &= s->read(target.m_Geom);
    ok &= s->read(target.m_Cape);
    ok &= s->read(target.m_AltPiv);
    ok &= s->read(target.m_AnimPiv);
    ok &= s->read(target.m_Part);
    ok &= s->read(target.m_Anim);
    ok &= s->read(target.m_SetState);
    ok &= s->read(target.m_ChildFx);
    ok &= s->read(target.m_Magnet);
    ok &= s->read(target.m_LookAt);
    ok &= s->read(target.m_PMagnet);
    ok &= s->read(target.m_POther);
    ok &= s->handleI24StructArray(target.m_Splats);
    ok &= s->handleI24StructArray(target.m_Sounds);
    ok &= s->read(target.m_SoundNoRepeat);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_LifeSpanJitter);
    ok &= s->read(target.m_Power);
    ok &= s->read(target.m_While);
    ok &= s->read(target.m_Until);
    ok &= s->read(target.m_WorldGroup);
    ok &= s->readEnum(target.m_Flags);
    return ok;
}

static bool loadFromI24(BinStore *s, FxInfo_Condition &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_On);
    ok &= s->read(target.m_Time);
    ok &= s->read(target.m_DayStart);
    ok &= s->read(target.m_DayEnd);
    ok &= s->read(target.m_Dist);
    ok &= s->read(target.m_Chance);
    ok &= s->read(target.DoMany);
    ok &= s->read(target.m_Repeat);
    ok &= s->read(target.m_RepeatJitter);
    ok &= s->read(target.str_TriggerBits);
    ok &= s->handleI24StructArray(target.m_Event);
    ok &= s->read(target.m_Random);
    ok &= s->read(target.m_ForceThreshold);
    return ok;
}

static bool loadFromI24(BinStore *s, FxInfo_Input &target)
{
    s->prepare();
    bool ok = true;
    return s->read(target.m_InpName);
}

static bool loadFromI24(BinStore *s, FxInfo &target)
{
    s->prepare();
    bool ok = true;

    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_FileAge);
    ok &= s->read(target.m_LifeSpan);
    ok &= s->read(target.m_Lighting);
    ok &= s->handleI24StructArray(target.m_Inputs);
    ok &= s->handleI24StructArray(target.m_Conditions);
    ok &= s->readEnum(target.m_Flags);
    ok &= s->read(target.m_PerformanceRadius);
    ok &= s->read(target.m_OnForceRadius);
    ok &= s->read(target.m_AnimScale);
    ok &= s->read(target.m_ClampMinScale);
    ok &= s->read(target.m_ClampMaxScale);
    return ok;
}

bool loadFrom(BinStore *s, Fx_AllInfos &target)
{
    s->prepare();
    if (s->isI24Data())
    {
        return s->handleI24StructArray(target);
    }
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("FxInfo" == _name)
        {
            target.emplace_back();
            ok &= loadFrom(s, target.back());
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
template <class Archive> static void serialize(Archive &archive, EventSplat &m)
{
    archive(cereal::make_nvp("Tex1", m.tex1));
    archive(cereal::make_nvp("Tex2", m.tex2));
}
template <class Archive> static void serialize(Archive &archive, FxSoundData &m)
{
    archive(cereal::make_nvp("Name", m.m_Name));
    // TODO: make the following fields optional
    archive(cereal::make_nvp("Radius", m.m_Radius));
    archive(cereal::make_nvp("Fade", m.m_Fade));
    archive(cereal::make_nvp("Volume", m.m_Volume));
}
template <class Archive> static void serialize(Archive &archive, FxGeoEntry_Event &m)
{
    archive(cereal::make_nvp("EName", m.m_EName));
    archive(cereal::make_nvp("Type", m.m_Type));
    archive(cereal::make_nvp("Inherit", m.m_Inherit));
    archive(cereal::make_nvp("Update", m.m_Update));
    archive(cereal::make_nvp("At", m.m_At));
    archive(cereal::make_nvp("Bhvr", m.m_Bhvr));
    archive(cereal::make_nvp("BhvrOverride", m.m_BhvrOverride));
    archive(cereal::make_nvp("JEvent", m.m_JEvent));
    archive(cereal::make_nvp("CEvent", m.m_CEvent));
    archive(cereal::make_nvp("CDestroy", m.m_CDestroy));
    archive(cereal::make_nvp("JDestroy", m.m_JDestroy));
    archive(cereal::make_nvp("CRotation", m.m_CRotation));
    archive(cereal::make_nvp("ParentVelocityFraction", m.m_ParentVelocityFraction));
    archive(cereal::make_nvp("CThresh", m.m_CThresh));
    archive(cereal::make_nvp("HardwareOnly", m.m_HardwareOnly));
    archive(cereal::make_nvp("SoftwareOnly", m.m_SoftwareOnly));
    archive(cereal::make_nvp("PhysicsOnly", m.m_PhysicsOnly));
    archive(cereal::make_nvp("CameraSpace", m.m_CameraSpace));
    archive(cereal::make_nvp("RayLength", m.m_RayLength));
    archive(cereal::make_nvp("AtRayFx", m.m_AtRayFx));
    archive(cereal::make_nvp("Geom", m.m_Geom));
    archive(cereal::make_nvp("Cape", m.m_Cape));
    archive(cereal::make_nvp("AltPiv", m.m_AltPiv));
    archive(cereal::make_nvp("AnimPiv", m.m_AnimPiv));
    archive(cereal::make_nvp("Part", m.m_Part));
    archive(cereal::make_nvp("Anim", m.m_Anim));
    archive(cereal::make_nvp("SetState", m.m_SetState));
    archive(cereal::make_nvp("ChildFx", m.m_ChildFx));
    archive(cereal::make_nvp("Magnet", m.m_Magnet));
    archive(cereal::make_nvp("LookAt", m.m_LookAt));
    archive(cereal::make_nvp("PMagnet", m.m_PMagnet));
    archive(cereal::make_nvp("POther", m.m_POther));
    archive(cereal::make_nvp("Splats", m.m_Splats));
    archive(cereal::make_nvp("Sounds", m.m_Sounds));
    archive(cereal::make_nvp("SoundNoRepeat", m.m_SoundNoRepeat));
    archive(cereal::make_nvp("LifeSpan", m.m_LifeSpan));
    archive(cereal::make_nvp("LifeSpanJitter", m.m_LifeSpanJitter));
    archive(cereal::make_nvp("Power", m.m_Power));
    archive(cereal::make_nvp("While", m.m_While));
    archive(cereal::make_nvp("Until", m.m_Until));
    archive(cereal::make_nvp("WorldGroup", m.m_WorldGroup));
    archive(cereal::make_nvp("Flags", m.m_Flags));
}

template <class Archive> static void serialize(Archive &archive, FxInfo_Condition &m)
{
    archive(cereal::make_nvp("On", m.m_On));
    archive(cereal::make_nvp("Time", m.m_Time));
    archive(cereal::make_nvp("DayStart", m.m_DayStart));
    archive(cereal::make_nvp("DayEnd", m.m_DayEnd));
    archive(cereal::make_nvp("Dist", m.m_Dist));
    archive(cereal::make_nvp("Chance", m.m_Chance));
    archive(cereal::make_nvp("DoMany", m.DoMany));
    archive(cereal::make_nvp("Repeat", m.m_Repeat));
    archive(cereal::make_nvp("RepeatJitter", m.m_RepeatJitter));
    archive(cereal::make_nvp("str_TriggerBits", m.str_TriggerBits));
    archive(cereal::make_nvp("Event", m.m_Event));
    archive(cereal::make_nvp("Random", m.m_Random));
    archive(cereal::make_nvp("ForceThreshold", m.m_ForceThreshold));
}
template <class Archive> static void serialize(Archive &archive, FxInfo &m)
{
    archive(cereal::make_nvp("Name", m.m_Name));
    archive(cereal::make_nvp("FileAge", m.m_FileAge));
    archive(cereal::make_nvp("LifeSpan", m.m_LifeSpan));
    archive(cereal::make_nvp("Lighting", m.m_Lighting));
    archive(cereal::make_nvp("Inputs", m.m_Inputs));
    archive(cereal::make_nvp("Conditions", m.m_Conditions));
    archive(cereal::make_nvp("Flags", m.m_Flags));
    archive(cereal::make_nvp("PerformanceRadius", m.m_PerformanceRadius));
    archive(cereal::make_nvp("OnForceRadius", m.m_OnForceRadius));
    archive(cereal::make_nvp("AnimScale", m.m_AnimScale));
    archive(cereal::make_nvp("ClampMinScale", m.m_ClampMinScale));
    archive(cereal::make_nvp("ClampMaxScale", m.m_ClampMaxScale));
}
template <class Archive> static void serialize(Archive &archive, FxInfo_Input &m)
{
    archive(cereal::make_nvp("InpName", m.m_InpName));
}
void saveTo(const Fx_AllInfos &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target, "AllFxInfos_Data", baseName, text_format);
}
bool loadFrom(const QString &filepath, Fx_AllInfos &target)
{
    QFSWrapper wrap;
    return commonReadFrom(wrap, filepath, "AllFxInfos_Data", target);
}
bool LoadFxInfoData(const QString &fname, Fx_AllInfos &infos)
{
    QFSWrapper wrap;
    BinStore   binfile;

    if (fname.contains(".crl"))
    {
        if (!loadFrom(fname, infos))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    if (!binfile.open(wrap, fname, fxbehaviors_i0_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if (!loadFrom(&binfile, infos))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
}

bool loadFrom(BinStore *s, Fx_AllBehaviors &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    if (s->end_encountered())
        return ok;
    QByteArray _name;
    while (s->nesting_name(_name))
    {
        s->nest_in();
        if ("Behavior" == _name)
        {
            target.emplace_back();
            ok &= loadFrom(s, target.back());
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
template <class Archive> void serialize(Archive &archive, ColorFx &m)
{
    archive(cereal::make_nvp("StartColor", m.startcolor));
    archive(cereal::make_nvp("StartTime", m.startTime));
}

template <class Archive> static void serialize(Archive &archive, FxBehavior &m)
{
    archive(cereal::make_nvp("ParamBitfield", m.m_ParamBitfield));
    archive(cereal::make_nvp("Name", m.m_Name));
    archive(cereal::make_nvp("FileAge", m.m_FileAge));
    archive(cereal::make_nvp("StartJitter", m.m_StartJitter));
    archive(cereal::make_nvp("InitialVelocity", m.m_InitialVelocity));
    archive(cereal::make_nvp("InitialVelocityJitter", m.m_InitialVelocityJitter));
    archive(cereal::make_nvp("InitialVelocityAngle", m.m_InitialVelocityAngle));
    archive(cereal::make_nvp("Gravity", m.m_Gravity));
    archive(cereal::make_nvp("Physics", m.m_Physics));
    archive(cereal::make_nvp("PhysRadius", m.m_PhysRadius));
    archive(cereal::make_nvp("PhysGravity", m.m_PhysGravity));
    archive(cereal::make_nvp("PhysRestitution", m.m_PhysRestitution));
    archive(cereal::make_nvp("PhysSFriction", m.m_PhysSFriction));
    archive(cereal::make_nvp("PhysDFriction", m.m_PhysDFriction));
    archive(cereal::make_nvp("PhysKillBelowSpeed", m.m_PhysKillBelowSpeed));
    archive(cereal::make_nvp("PhysDensity", m.m_PhysDensity));
    archive(cereal::make_nvp("PhysForceRadius", m.m_PhysForceRadius));
    archive(cereal::make_nvp("PhysForcePower", m.m_PhysForcePower));
    archive(cereal::make_nvp("PhysForcePowerJitter", m.m_PhysForcePowerJitter));
    archive(cereal::make_nvp("PhysForceCentripetal", m.m_PhysForceCentripetal));
    archive(cereal::make_nvp("PhysForceSpeedScaleMax", m.m_PhysForceSpeedScaleMax));
    archive(cereal::make_nvp("PhysScale", m.m_PhysScale));

    archive(cereal::make_nvp("PhysJointDOFs", m.m_PhysJointDOFs));
    archive(cereal::make_nvp("PhysJointAnchor", m.m_PhysJointAnchor));
    archive(cereal::make_nvp("PhysJointAngLimit", m.m_PhysJointAngLimit));
    archive(cereal::make_nvp("PhysJointLinLimit", m.m_PhysJointLinLimit));
    archive(cereal::make_nvp("PhysJointBreakTorque", m.m_PhysJointBreakTorque));
    archive(cereal::make_nvp("PhysJointBreakForce", m.m_PhysJointBreakForce));
    archive(cereal::make_nvp("PhysJointLinSpring", m.m_PhysJointLinSpring));
    archive(cereal::make_nvp("PhysJointLinSpringDamp", m.m_PhysJointLinSpringDamp));
    archive(cereal::make_nvp("PhysJointAngSpring", m.m_PhysJointAngSpring));
    archive(cereal::make_nvp("PhysJointAngSpringDamp", m.m_PhysJointAngSpringDamp));
    archive(cereal::make_nvp("PhysJointDrag", m.m_PhysJointDrag));
    archive(cereal::make_nvp("PhysJointCollidesWorld", m.m_PhysJointCollidesWorld));
    archive(cereal::make_nvp("PhysForceType", m.m_PhysForceType));
    archive(cereal::make_nvp("PhysDebris", m.m_PhysDebris));
    archive(cereal::make_nvp("Spin", m.m_Spin));
    archive(cereal::make_nvp("SpinJitter", m.m_SpinJitter));
    archive(cereal::make_nvp("FadeInLength", m.m_FadeInLength));
    archive(cereal::make_nvp("FadeOutLength", m.m_FadeOutLength));
    archive(cereal::make_nvp("Shake", m.m_Shake));
    archive(cereal::make_nvp("ShakeFallOff", m.m_ShakeFallOff));
    archive(cereal::make_nvp("ShakeRadius", m.m_ShakeRadius));
    archive(cereal::make_nvp("Blur", m.m_Blur));
    archive(cereal::make_nvp("BlurFallOff", m.m_BlurFallOff));
    archive(cereal::make_nvp("BlurRadius", m.m_BlurRadius));
    archive(cereal::make_nvp("Scale", m.m_Scale));
    archive(cereal::make_nvp("ScaleRate", m.m_ScaleRate));
    archive(cereal::make_nvp("ScaleTime", m.m_ScaleTime));
    archive(cereal::make_nvp("EndScale", m.m_EndScale));
    archive(cereal::make_nvp("Stretch", m.m_Stretch));
    archive(cereal::make_nvp("Drag", m.m_Drag));
    archive(cereal::make_nvp("PyrRotate", m.m_PyrRotate));
    archive(cereal::make_nvp("PyrRotateJitter", m.m_PyrRotateJitter));
    archive(cereal::make_nvp("PositionOffset", m.m_PositionOffset));
    archive(cereal::make_nvp("UseShieldOffset", m.m_UseShieldOffset));
    archive(cereal::make_nvp("TrackRate", m.m_TrackRate));
    archive(cereal::make_nvp("TrackMethod", m.m_TrackMethod));
    archive(cereal::make_nvp("Collides", m.m_Collides));
    archive(cereal::make_nvp("LifeSpan", m.m_LifeSpan));
    archive(cereal::make_nvp("AnimScale", m.m_AnimScale));
    archive(cereal::make_nvp("Alpha", m.m_Alpha));
    archive(cereal::make_nvp("PulsePeakTime", m.m_PulsePeakTime));
    archive(cereal::make_nvp("PulseBrightness", m.m_PulseBrightness));
    archive(cereal::make_nvp("PulseClamp", m.m_PulseClamp));
    archive(cereal::make_nvp("SplatFlags", m.m_SplatFlags));
    archive(cereal::make_nvp("SplatFalloffType", m.m_SplatFalloffType));
    archive(cereal::make_nvp("SplatNormalFade", m.m_SplatNormalFade));
    archive(cereal::make_nvp("SplatFadeCenter", m.m_SplatFadeCenter));
    archive(cereal::make_nvp("SplatSetBack", m.m_SplatSetBack));
    archive(cereal::make_nvp("StAnim", m.m_StAnim));
    archive(cereal::make_nvp("HueShift", m.m_HueShift));
    archive(cereal::make_nvp("HueShiftJitter", m.m_HueShiftJitter));
    archive(cereal::make_nvp("ColorEffects", m.m_ColorEffect));

    archive(cereal::make_nvp("InheritGroupTint", m.m_InheritGroupTint));
    archive(cereal::make_nvp("TintGeom", m.m_TintGeom));
}

void saveTo(const Fx_AllBehaviors &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target, "Fx_AllBehaviors", baseName, text_format);
}
bool loadFrom(const QString &filepath, Fx_AllBehaviors &target)
{
    QFSWrapper wrap;
    return commonReadFrom(wrap, filepath, "Fx_AllBehaviors", target);
}
bool LoadFxBehaviorData(const QString &fname, Fx_AllBehaviors &behaviors)
{
    QFSWrapper wrap;
    BinStore   binfile;

    if (fname.contains(".crl"))
    {
        if (!loadFrom(fname, behaviors))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    if (!binfile.open(wrap, fname, fxbehaviors_i0_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if (!loadFrom(&binfile, behaviors))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
}
