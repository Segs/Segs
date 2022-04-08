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

#include "attrib_serializers.h"
#include "GameData/CharacterAttributes.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "DataStorage.h"

namespace
{
    bool loadFrom(BinStore * s, Parse_AttribDesc & target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.Name);
        ok &= s->read(target.DisplayName);
        ok &= s->read(target.IconName);
        ok &= s->prepare_nested(); // will update the file size left
        assert(s->end_encountered());
        return ok;
    }
}
static bool loadFromI24(BinStore * s, Parse_AttribDesc & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.Name);
    ok &= s->read(target.DisplayName);
    ok &= s->read(target.IconName);
    return ok;
}
bool loadFrom(BinStore * s, AttribNames_Data & target)
{
    bool ok = true;
    s->prepare();
    if(s->isI24Data()) {
        ok &= s->handleI24StructArray(target.m_Damage);
        ok &= s->handleI24StructArray(target.m_Defense);
        ok &= s->handleI24StructArray(target.m_Boost);
        ok &= s->handleI24StructArray(target.m_Group);
        ok &= s->handleI24StructArray(target.m_Mode);
        ok &= s->handleI24StructArray(target.m_Elusivity);
        ok &= s->handleI24StructArray(target.m_StackKeys);
        return ok;
    }
    ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Damage"==_name) {
            target.m_Damage.emplace_back();
            ok &= loadFrom(s,target.m_Damage.back());
        } else if("Defense"==_name) {
            target.m_Defense.emplace_back();
            ok &= loadFrom(s,target.m_Defense.back());
        } else if("Boost"==_name) {
            target.m_Boost.emplace_back();
            ok &= loadFrom(s,target.m_Boost.back());
        } else if("Group"==_name) {
            target.m_Group.emplace_back();
            ok &= loadFrom(s,target.m_Group.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

void saveTo(const AttribNames_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AttributeNames",baseName,text_format);
}

bool loadFrom(BinStore *s, Parse_CharAttrib &target)
{
    s->prepare();

    bool ok = true;
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_JumpHeight);
    s->read(target.m_MovementControl);
    s->read(target.m_MovementFriction);
    s->read(target.m_Stealth);
    s->read(target.m_StealthRadius);
    s->read(target.m_PerceptionRadius);
    s->read(target.m_Regeneration);
    s->read(target.m_Recovery);
    s->read(target.m_ThreatLevel);
    s->read(target.m_Taunt);
    s->read(target.m_Confused);
    s->read(target.m_Afraid);
    s->read(target.m_Held);
    s->read(target.m_Immobilized);
    s->read(target.m_IsStunned);
    s->read(target.m_Sleep);
    s->read(target.m_IsFlying);
    s->read(target.m_HasJumppack);
    s->read(target.m_Teleport);
    s->read(target.m_Untouchable);
    s->read(target.m_Intangible);
    s->read(target.m_OnlyAffectsSelf);
    s->read(target.m_Knockup);
    s->read(target.m_Knockback);
    s->read(target.m_Repel);
    s->read(target.m_Accuracy);
    s->read(target.m_Radius);
    s->read(target.m_Arc);
    s->read(target.m_Range);
    s->read(target.m_TimeToActivate);
    s->read(target.m_RechargeTime);
    s->read(target.m_InterruptTime);
    s->read(target.m_EnduranceDiscount);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, Parse_CharAttribMax &target)
{
    s->prepare();

    bool ok = true;
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_JumpHeight);
    s->read(target.m_MovementControl);
    s->read(target.m_MovementFriction);
    s->read(target.m_Stealth);
    s->read(target.m_StealthRadius);
    s->read(target.m_PerceptionRadius);
    s->read(target.m_Regeneration);
    s->read(target.m_Recovery);
    s->read(target.m_ThreatLevel);
    s->read(target.m_Taunt);
    s->read(target.m_Confused);
    s->read(target.m_Afraid);
    s->read(target.m_Held);
    s->read(target.m_Immobilized);
    s->read(target.m_IsStunned);
    s->read(target.m_Sleep);
    s->read(target.m_IsFlying);
    s->read(target.m_HasJumppack);
    s->read(target.m_Teleport);
    s->read(target.m_Untouchable);
    s->read(target.m_Intangible);
    s->read(target.m_OnlyAffectsSelf);
    s->read(target.m_Knockup);
    s->read(target.m_Knockback);
    s->read(target.m_Repel);
    s->read(target.m_Accuracy);
    s->read(target.m_Radius);
    s->read(target.m_Arc);
    s->read(target.m_Range);
    s->read(target.m_TimeToActivate);
    s->read(target.m_RechargeTime);
    s->read(target.m_InterruptTime);
    s->read(target.m_EnduranceDiscount);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFromI24(BinStore *s, Parse_CharAttribMax &target)
{
    s->prepare();

    bool ok = true;
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Absorb);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_Insight);
    ok &= s->read(target.m_Rage);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }
    ok &= s->read(target.m_Defense);
    ok &= s->read(target.m_SpeedRunning);
    ok &= s->read(target.m_SpeedFlying);
    ok &= s->read(target.m_SpeedSwimming);
    ok &= s->read(target.m_SpeedJumping);
    ok &= s->read(target.m_JumpHeight);
    ok &= s->read(target.m_MovementControl);
    ok &= s->read(target.m_MovementFriction);
    ok &= s->read(target.m_Stealth);
    ok &= s->read(target.m_StealthRadius);
    ok &= s->read(target.m_StealthRadiusPlayer);
    ok &= s->read(target.m_PerceptionRadius);
    ok &= s->read(target.m_Regeneration);
    ok &= s->read(target.m_Recovery);
    ok &= s->read(target.m_InsightRecovery);
    ok &= s->read(target.m_ThreatLevel);
    ok &= s->read(target.m_Taunt);
    ok &= s->read(target.m_Placate);
    ok &= s->read(target.m_Confused);
    ok &= s->read(target.m_Afraid);
    ok &= s->read(target.m_Terrorized);
    ok &= s->read(target.m_Held);
    ok &= s->read(target.m_Immobilized);
    ok &= s->read(target.m_IsStunned);
    ok &= s->read(target.m_Sleep);
    ok &= s->read(target.m_IsFlying);
    ok &= s->read(target.m_HasJumppack);
    ok &= s->read(target.m_Teleport);
    ok &= s->read(target.m_Untouchable);
    ok &= s->read(target.m_Intangible);
    ok &= s->read(target.m_OnlyAffectsSelf);
    ok &= s->read(target.m_ExperienceGain);
    ok &= s->read(target.m_InfluenceGain);
    ok &= s->read(target.m_PrestigeGain);
    ok &= s->read(target.m_Evade);
    ok &= s->read(target.m_Knockup);
    ok &= s->read(target.m_Knockback);
    ok &= s->read(target.m_Repel);
    ok &= s->read(target.m_Accuracy);
    ok &= s->read(target.m_Radius);
    ok &= s->read(target.m_Arc);
    ok &= s->read(target.m_Range);
    ok &= s->read(target.m_TimeToActivate);
    ok &= s->read(target.m_RechargeTime);
    ok &= s->read(target.m_InterruptTime);
    ok &= s->read(target.m_EnduranceDiscount);
    ok &= s->read(target.m_InsightDiscount);
    ok &= s->read(target.m_Meter);
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_ElusivityTypes[i]);
    }
    ok &= s->read(target.m_ElusivityBase);
    return ok;

}
void serializeToDb(const Parse_CharAttrib &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(Parse_CharAttrib &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}

//! @}

bool loadFromI24(BinStore *s, Parse_CharAttrib &target)
{
    bool ok = true;
    s->prepare();
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Absorb);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_Insight);
    ok &= s->read(target.m_Rage);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }
    ok &= s->read(target.m_Defense);
    ok &= s->read(target.m_SpeedRunning);
    ok &= s->read(target.m_SpeedFlying);
    ok &= s->read(target.m_SpeedSwimming);
    ok &= s->read(target.m_SpeedJumping);
    ok &= s->read(target.m_JumpHeight);
    ok &= s->read(target.m_MovementControl);
    ok &= s->read(target.m_MovementFriction);
    ok &= s->read(target.m_Stealth);
    ok &= s->read(target.m_StealthRadius);
    ok &= s->read(target.m_StealthRadiusPlayer);
    ok &= s->read(target.m_PerceptionRadius);
    ok &= s->read(target.m_Regeneration);
    ok &= s->read(target.m_Recovery);
    ok &= s->read(target.m_InsightRecovery);
    ok &= s->read(target.m_ThreatLevel);
    ok &= s->read(target.m_Taunt);
    ok &= s->read(target.m_Placate);
    ok &= s->read(target.m_Confused);
    ok &= s->read(target.m_Afraid);
    ok &= s->read(target.m_Terrorized);
    ok &= s->read(target.m_Held);
    ok &= s->read(target.m_Immobilized);
    ok &= s->read(target.m_IsStunned);
    ok &= s->read(target.m_Sleep);
    ok &= s->read(target.m_IsFlying);
    ok &= s->read(target.m_HasJumppack);
    ok &= s->read(target.m_Teleport);
    ok &= s->read(target.m_Untouchable);
    ok &= s->read(target.m_Intangible);
    ok &= s->read(target.m_OnlyAffectsSelf);
    ok &= s->read(target.m_ExperienceGain);
    ok &= s->read(target.m_InfluenceGain);
    ok &= s->read(target.m_PrestigeGain);
    ok &= s->read(target.m_Evade);
    ok &= s->read(target.m_Knockup);
    ok &= s->read(target.m_Knockback);
    ok &= s->read(target.m_Repel);
    ok &= s->read(target.m_Accuracy);
    ok &= s->read(target.m_Radius);
    ok &= s->read(target.m_Arc);
    ok &= s->read(target.m_Range);
    ok &= s->read(target.m_TimeToActivate);
    ok &= s->read(target.m_RechargeTime);
    ok &= s->read(target.m_InterruptTime);
    ok &= s->read(target.m_EnduranceDiscount);
    ok &= s->read(target.m_InsightDiscount);
    ok &= s->read(target.m_Meter);
    for(int i=0; i<20; ++i) {
        ok &= s->read(target.m_ElusivityTypes[i]);
    }
    ok &= s->read(target.m_ElusivityBase);
    return ok;
}
