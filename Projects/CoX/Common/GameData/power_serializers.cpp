/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "power_serializers.h"
#include "power_definitions.h"
#include "seq_definitions.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "DataStorage.h"

#include <QMetaEnum>
#include <type_traits>

bool loadFrom(BinStore *s, StoredAttribMod &target)
{
    bool ok = true;
    s->prepare();
    // Unused/unset fields:  TogglePower / FXName
    ok &= s->read(target.name);
    ok &= s->read(target.DisplayAttackerHit);
    ok &= s->read(target.DisplayVictimHit);
    ok &= s->read(target.Attrib);
    ok &= s->readEnum(target.Aspect);
    ok &= s->readEnum(target.Target);
    ok &= s->read(target.Table);

    ok &= s->read(target.Scale);
    ok &= s->readEnum(target.Type);
    ok &= s->read(target.Delay);
    ok &= s->read(target.Period);
    ok &= s->read(target.Chance);

    ok &= s->read(target.CancelOnMiss);     // T/F
    ok &= s->read(target.NearGround);       // T/F
    ok &= s->read(target.AllowStrength);    // T/F
    ok &= s->read(target.AllowResistance); // T/F
    ok &= s->readEnum(target.StackType);
    ok &= s->read(target.Duration);
    ok &= s->read(target.Magnitude);
    ok &= s->read(target.ContinuingBits);
    ok &= s->read(target.ContinuingFX);
    ok &= s->read(target.ConditionalBits);
    ok &= s->read(target.ConditionalFX);
    ok &= s->read(target.EntityDef);
    ok &= s->read(target.PriorityListDefense);
    ok &= s->read(target.PriorityListOffense);
    ok &= s->read(target.PriorityListPassive);
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, Power_Data &target)
{
    bool ok = true;
    s->prepare();
    // Unused/unset fields:  TogglePower / FXName
    ok &= s->read(target.m_Name);
    ok &= s->read(target.DisplayName);
    ok &= s->read(target.DisplayHelp);
    ok &= s->read(target.DisplayShortHelp);
    ok &= s->read(target.DisplayAttackerAttack);
    ok &= s->read(target.DisplayAttackerHit);
    ok &= s->read(target.DisplayVictimHit);
    ok &= s->read(target.IconName);
    ok &= s->readEnum(target.Type);
    ok &= s->readEnum(target.AttackTypes);
    ok &= s->read(target.Requires);
    ok &= s->read(target.Accuracy);
    ok &= s->read(target.NearGround);
    ok &= s->read(target.TargetNearGround);
    ok &= s->read(target.CastableAfterDeath);
    ok &= s->read(target.AIReport);
    ok &= s->readEnum(target.EffectArea);
    ok &= s->read(target.Radius);
    ok &= s->read(target.Arc);
    ok &= s->read(target.Range);
    ok &= s->read(target.RangeSecondary);
    ok &= s->read(target.TimeToActivate);
    ok &= s->read(target.RechargeTime);
    ok &= s->read(target.ActivatePeriod);
    ok &= s->read(target.EnduranceCost);
    ok &= s->read(target.DestroyOnLimit); //true/false
    ok &= s->read(target.m_NumCharges);
    ok &= s->read(target.m_UsageTime);
    ok &= s->read(target.m_Lifetime);
    ok &= s->read(target.InterruptTime);
    ok &= s->readEnum(target.TargetVisibility);
    ok &= s->readEnum(target.Target);
    ok &= s->readEnum(target.TargetSecondary);
    ok &= s->readEnum(target.EntsAutoHit);
    ok &= s->readEnum(target.EntsAffected);
    ok &= s->read(target.BoostsAllowed);
    ok &= s->read(target.GroupMembership);
    ok &= s->read(target.AIGroups);
    ok &= s->read(target.IgnoreStrength); // true/false
    ok &= s->readEnum(target.ModeSeqBits);
    ok &= s->readEnum(target.AttackBits); // or cast anim bits
    ok &= s->readEnum(target.BlockBits);
    ok &= s->readEnum(target.WindUpBits);
    ok &= s->readEnum(target.HitBits);
    ok &= s->readEnum(target.DeathBits); // or DeathAnimBits - resolve using g_hashTabPtr
    ok &= s->readEnum(target.ActivationBits); // or AttachedAnim - resolve using g_hashTabPtr
    ok &= s->read(target.ActivationFX); // or AttachedFxName
    ok &= s->read(target.AttackFX); // or TravellingProjectileEffect
    ok &= s->read(target.HitFX); // or AttachedToVictimFxName
    ok &= s->read(target.m_FramesBeforeHit); // or a punch hits, or a missile spawns
    ok &= s->read(target.DelayedHit); // True/False
    ok &= s->read(target.WindUpFX);
    ok &= s->read(target.BlockFX);
    ok &= s->read(target.DeathFX);
    ok &= s->read(target.m_AttackFrames); // those are non-inerruptible ?
    ok &= s->read(target.InitialAttackFX);
    ok &= s->readEnum(target.InitialAttackBits); //each entry can be mapped from int to attack bit name using g_hashTabPtr ?
    ok &= s->read(target.m_InitialFramesBeforeHit);
    ok &= s->read(target.ProjectileSpeed);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("AttribMod"==_name) {
            target.pAttribMod.emplace_back();
            ok &= loadFrom(s,target.pAttribMod.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, Parse_PowerSet &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.DisplayName);
    ok &= s->read(target.DisplayHelp);
    ok &= s->read(target.DisplayShortHelp);
    ok &= s->read(target.IconName);
    ok &= s->read(target.Available);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Power"==_name) {
            target.m_Powers.emplace_back();
            ok &= loadFrom(s,target.m_Powers.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, StoredPowerCategory &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.disp_name);
    ok &= s->read(target.disp_help);
    ok &= s->read(target.disp_short_help);
    // Unused/unset field 'Available'
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("PowerSet"==_name) {
            target.m_PowerSets.emplace_back();
            ok &= loadFrom(s,target.m_PowerSets.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, AllPowerCategories &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("PowerCategory"==_name) {
            target.m_categories.emplace_back();
            ok &= loadFrom(s,target.m_categories.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

/////////////////////////////////////////////////////////////
/// Cereal specific code
///
//template <class Archive>
//std::string save_minimal(Archive const &, log_level_t const & obj)
//{
//    switch (obj)
//    {
//        case log_level_t::message: return "message";
//        case log_level_t::warning: return "warning";
//        case log_level_t::error: return "error";
//    }
//    return "message"; // Default value
//}

//template <class Archive> void load_minimal(Archive const &, log_level_t & obj, std::string const & value)
//{
//    if(value == "message") obj = log_level_t::message;
//    else if(value == "warning") obj = log_level_t::warning;
//    else if(value == "error") obj = log_level_t::error;
//    else obj = log_level_t::message; // Default value
//}
template<class Archive>
static void serialize(Archive & archive, StoredAttribMod & src)
{

    archive(cereal::make_nvp("Name",src.name));
    archive(cereal::make_nvp("DisplayAttackerHit",src.DisplayAttackerHit));
    archive(cereal::make_nvp("DisplayVictimHit",src.DisplayVictimHit));
    archive(cereal::make_nvp("Attrib",src.Attrib));
    archive(cereal::make_nvp("Aspect",src.Aspect));
    archive(cereal::make_nvp("Target",src.Target));
    archive(cereal::make_nvp("Table",src.Table));

    archive(cereal::make_nvp("Scale",src.Scale));
    archive(cereal::make_nvp("Type",src.Type));
    archive(cereal::make_nvp("Delay",src.Delay));
    archive(cereal::make_nvp("Period",src.Period));
    archive(cereal::make_nvp("Chance",src.Chance));

    archive(cereal::make_nvp("CancelOnMiss",src.CancelOnMiss));     // T/F
    archive(cereal::make_nvp("NearGround",src.NearGround));       // T/F
    archive(cereal::make_nvp("AllowStrength",src.AllowStrength));    // T/F
    archive(cereal::make_nvp("AllowResistance",src.AllowResistance)); // T/F
    archive(cereal::make_nvp("StackType",src.StackType));
    archive(cereal::make_nvp("Duration",src.Duration));
    archive(cereal::make_nvp("Magnitude",src.Magnitude));
    archive(cereal::make_nvp("ContinuingBits",src.ContinuingBits));
    archive(cereal::make_nvp("ContinuingFX",src.ContinuingFX));
    archive(cereal::make_nvp("ConditionalBits",src.ConditionalBits));
    archive(cereal::make_nvp("ConditionalFX",src.ConditionalFX));
    archive(cereal::make_nvp("EntityDef",src.EntityDef));
    archive(cereal::make_nvp("PriorityListDefense",src.PriorityListDefense));
    archive(cereal::make_nvp("PriorityListOffense",src.PriorityListOffense));
    archive(cereal::make_nvp("PriorityListPassive",src.PriorityListPassive));
}
template<class Archive>
static void serialize(Archive & archive, SeqBitNames & src)
{
    auto val = std::underlying_type<SeqBitNames>::type(src);
    archive(val);
    src = SeqBitNames(val);
}
namespace cereal
{
static std::string save_minimal(cereal::JSONOutputArchive & /*archive*/, const SeqBitNames & src)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SEGS_Enums::SeqBitNames>();
    QString val = metaEnum.valueToKey(std::underlying_type<SeqBitNames>::type(src));
    return val.toStdString();
}
static void load_minimal(const cereal::JSONInputArchive & /*archive*/, SeqBitNames & val,const std::string &src)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SEGS_Enums::SeqBitNames>();
    val  = SeqBitNames(metaEnum.keyToValue(src.c_str()));
}
static std::string save_minimal(cereal::JSONOutputArchive & /*archive*/, const AttackType & src)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SEGS_Enums_Power::AttackType>();
    QString val = metaEnum.valueToKey(std::underlying_type<AttackType>::type(src));
    return val.toStdString();
}
static void load_minimal(const cereal::JSONInputArchive & /*archive*/, AttackType & val,const std::string &src)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SEGS_Enums_Power::AttackType>();
    val  = AttackType(metaEnum.keyToValue(src.c_str()));
}
}
template<class Archive>
static void serialize(Archive & archive, Power_Data & src)
{
    archive(cereal::make_nvp("Name",src.m_Name));
    archive(cereal::make_nvp("DisplayName",src.DisplayName));
    archive(cereal::make_nvp("DisplayHelp",src.DisplayHelp));
    archive(cereal::make_nvp("DisplayShortHelp",src.DisplayShortHelp));
    archive(cereal::make_nvp("DisplayAttackerAttack",src.DisplayAttackerAttack));
    archive(cereal::make_nvp("DisplayAttackerHit",src.DisplayAttackerHit));
    archive(cereal::make_nvp("DisplayVictimHit",src.DisplayVictimHit));
    archive(cereal::make_nvp("IconName",src.IconName));
    archive(cereal::make_nvp("Type",src.Type));
    archive(cereal::make_nvp("AttackTypes",src.AttackTypes));
    archive(cereal::make_nvp("Requires",src.Requires));
    archive(cereal::make_nvp("Accuracy",src.Accuracy));
    archive(cereal::make_nvp("NearGround",src.NearGround));
    archive(cereal::make_nvp("TargetNearGround",src.TargetNearGround));
    archive(cereal::make_nvp("CastableAfterDeath",src.CastableAfterDeath));
    archive(cereal::make_nvp("AIReport",src.AIReport));
    archive(cereal::make_nvp("EffectArea",src.EffectArea));
    archive(cereal::make_nvp("Radius",src.Radius));
    archive(cereal::make_nvp("Arc",src.Arc));
    archive(cereal::make_nvp("Range",src.Range));
    archive(cereal::make_nvp("RangeSecondary",src.RangeSecondary));
    archive(cereal::make_nvp("TimeToActivate",src.TimeToActivate));
    archive(cereal::make_nvp("RechargeTime",src.RechargeTime));
    archive(cereal::make_nvp("ActivatePeriod",src.ActivatePeriod));
    archive(cereal::make_nvp("EnduranceCost",src.EnduranceCost));
    archive(cereal::make_nvp("DestroyOnLimit",src.DestroyOnLimit)); //true/false
    archive(cereal::make_nvp("NumCharges",src.m_NumCharges));
    archive(cereal::make_nvp("UsageTime",src.m_UsageTime));
    archive(cereal::make_nvp("Lifetime",src.m_Lifetime));
    archive(cereal::make_nvp("InterruptTime",src.InterruptTime));
    archive(cereal::make_nvp("TargetVisibility",src.TargetVisibility));
    archive(cereal::make_nvp("Target",src.Target));
    archive(cereal::make_nvp("TargetSecondary",src.TargetSecondary));
    archive(cereal::make_nvp("EntsAutoHit",src.EntsAutoHit));
    archive(cereal::make_nvp("EntsAffected",src.EntsAffected));
    archive(cereal::make_nvp("BoostsAllowed",src.BoostsAllowed));
    archive(cereal::make_nvp("GroupMembership",src.GroupMembership));
    archive(cereal::make_nvp("AIGroups",src.AIGroups));
    archive(cereal::make_nvp("IgnoreStrength",src.IgnoreStrength)); // true/false
    archive(cereal::make_nvp("ModeSeqBits",src.ModeSeqBits));
    archive(cereal::make_nvp("AttackBits",src.AttackBits)); // or cast anim bits
    archive(cereal::make_nvp("BlockBits",src.BlockBits));
    archive(cereal::make_nvp("WindUpBits",src.WindUpBits));
    archive(cereal::make_nvp("HitBits",src.HitBits));
    archive(cereal::make_nvp("DeathBits",src.DeathBits)); // or DeathAnimBits - resolve using g_hashTabPtr
    archive(cereal::make_nvp("ActivationBits",src.ActivationBits)); // or AttachedAnim - resolve using g_hashTabPtr
    archive(cereal::make_nvp("ActivationFX",src.ActivationFX)); // or AttachedFxName
    archive(cereal::make_nvp("AttackFX",src.AttackFX)); // or TravellingProjectileEffect
    archive(cereal::make_nvp("HitFX",src.HitFX)); // or AttachedToVictimFxName
    archive(cereal::make_nvp("FramesBeforeHit",src.m_FramesBeforeHit)); // or a punch hits, or a missile spawns
    archive(cereal::make_nvp("DelayedHit",src.DelayedHit)); // True/False
    archive(cereal::make_nvp("WindUpFX",src.WindUpFX));
    archive(cereal::make_nvp("BlockFX",src.BlockFX));
    archive(cereal::make_nvp("DeathFX",src.DeathFX));
    archive(cereal::make_nvp("AttackFrames",src.m_AttackFrames)); // those are non-inerruptible ?
    archive(cereal::make_nvp("InitialAttackFX",src.InitialAttackFX));
    archive(cereal::make_nvp("InitialAttackBits",src.InitialAttackBits)); //each entry can be mapped from int to attack bit name using g_hashTabPtr ?
    archive(cereal::make_nvp("InitialFramesBeforeHit",src.m_InitialFramesBeforeHit));
    archive(cereal::make_nvp("ProjectileSpeed",src.ProjectileSpeed));
    archive(cereal::make_nvp("AttribMods",src.pAttribMod));
}

template<class Archive>
static void serialize(Archive & archive, Parse_PowerSet & src)
{
    archive(cereal::make_nvp("Name",src.m_Name));
    archive(cereal::make_nvp("DisplayName",src.DisplayName));
    archive(cereal::make_nvp("DisplayHelp",src.DisplayHelp));
    archive(cereal::make_nvp("DisplayShortHelp",src.DisplayShortHelp));
    archive(cereal::make_nvp("IconName",src.IconName));
    archive(cereal::make_nvp("Available",src.Available));
    archive(cereal::make_nvp("Powers",src.m_Powers));
}

template<class Archive>
static void serialize(Archive & archive, StoredPowerCategory & src)
{
    archive(cereal::make_nvp("Name",src.name));
    archive(cereal::make_nvp("DispName",src.disp_name));
    archive(cereal::make_nvp("DispShortHelp",src.disp_short_help));
    archive(cereal::make_nvp("DispHelp",src.disp_help));
    archive(cereal::make_nvp("PowerSets",src.m_PowerSets));
}

template<class Archive>
static void serialize(Archive & archive, AllPowerCategories & src)
{
    archive(cereal::make_nvp("Categories",src.m_categories));
}

void saveTo(const AllPowerCategories & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"Powers",baseName,text_format);
}

//! @}
