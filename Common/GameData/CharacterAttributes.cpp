#include "CharacterAttributes.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <QDebug>

template<class Archive>
void Parse_AttribDesc::serialize(Archive & archive)
{
    archive(cereal::make_nvp("Name",Name));
    archive(cereal::make_nvp("DisplayName",DisplayName));
    archive(cereal::make_nvp("IconName",IconName));
}
SPECIALIZE_SERIALIZATIONS(Parse_AttribDesc)

template<class Archive>
void AttribNames_Data::serialize(Archive & archive)
{
    archive(cereal::make_nvp("Damage",m_Damage));
    archive(cereal::make_nvp("Defense",m_Defense));
    archive(cereal::make_nvp("Boost",m_Boost));
    archive(cereal::make_nvp("Group",m_Group));
}
SPECIALIZE_SERIALIZATIONS(AttribNames_Data)

template<class Archive>
void Parse_CharAttrib::serialize(Archive & archive)
{
    char buf[128];
    for(int i=0; i<24; ++i)
    {
        sprintf(buf,"DamageType%02d",i);
        archive(cereal::make_nvp(buf,m_DamageTypes[i]));
    }
    archive(cereal::make_nvp("HitPoints",m_HitPoints));
    archive(cereal::make_nvp("Endurance",m_Endurance));
    archive(cereal::make_nvp("ToHit",m_ToHit));
    for(int i=0; i<24; ++i) {
        sprintf(buf,"DefenseType00%02d",i);
        archive(cereal::make_nvp(buf,m_DefenseTypes[i]));
    }

    archive(cereal::make_nvp("Defense",m_Defense));
    archive(cereal::make_nvp("Evade",m_Evade));
    archive(cereal::make_nvp("SpeedRunning",m_SpeedRunning));
    archive(cereal::make_nvp("SpeedFlying",m_SpeedFlying));
    archive(cereal::make_nvp("SpeedSwimming",m_SpeedSwimming));
    archive(cereal::make_nvp("SpeedJumping",m_SpeedJumping));
    archive(cereal::make_nvp("JumpHeight",m_jump_height));
    archive(cereal::make_nvp("MovementControl",m_MovementControl));
    archive(cereal::make_nvp("MovementFriction",m_MovementFriction));
    archive(cereal::make_nvp("Stealth",m_Stealth));
    archive(cereal::make_nvp("StealthRadius",m_StealthRadius));
    archive(cereal::make_nvp("PerceptionRadius",m_PerceptionRadius));
    archive(cereal::make_nvp("Regeneration",m_Regeneration));
    archive(cereal::make_nvp("Recovery",m_Recovery));
    archive(cereal::make_nvp("ThreatLevel",m_ThreatLevel));
    archive(cereal::make_nvp("Taunt",m_Taunt));
    archive(cereal::make_nvp("Confused",m_Confused));
    archive(cereal::make_nvp("Afraid",m_Afraid));
    archive(cereal::make_nvp("Held",m_Held));
    archive(cereal::make_nvp("Immobilized",m_Immobilized));
    archive(cereal::make_nvp("Stunned",m_is_stunned));
    archive(cereal::make_nvp("Sleep",m_Sleep));
    archive(cereal::make_nvp("Fly",m_is_flying));
    archive(cereal::make_nvp("Jumppack",m_has_jumppack));
    archive(cereal::make_nvp("Teleport",m_Teleport));
    archive(cereal::make_nvp("Untouchable",m_Untouchable));
    archive(cereal::make_nvp("Intangible",m_Intangible));
    archive(cereal::make_nvp("OnlyAffectsSelf",m_OnlyAffectsSelf));
    archive(cereal::make_nvp("Knockup",m_Knockup));
    archive(cereal::make_nvp("Knockback",m_Knockback));
    archive(cereal::make_nvp("Repel",m_Repel));
    archive(cereal::make_nvp("Accuracy",m_Accuracy));
    archive(cereal::make_nvp("Radius",m_Radius));
    archive(cereal::make_nvp("Arc",m_Arc));
    archive(cereal::make_nvp("Range",m_Range));
    archive(cereal::make_nvp("TimeToActivate",m_TimeToActivate));
    archive(cereal::make_nvp("RechargeTime",m_RechargeTime));
    archive(cereal::make_nvp("InterruptTime",m_InterruptTime));
    archive(cereal::make_nvp("EnduranceDiscount",m_EnduranceDiscount));
}
SPECIALIZE_SERIALIZATIONS(Parse_CharAttrib)

template<class Archive>
void Parse_CharAttribMax::serialize(Archive & archive)
{
    char buf[128];
    for(int i=0; i<24; ++i)
    {
        sprintf(buf,"DamageType%02d",i);
        archive(cereal::make_nvp(buf,m_DamageTypes[i]));
    }
    archive(cereal::make_nvp("HitPoints",m_HitPoints));
    archive(cereal::make_nvp("Endurance",m_Endurance));
    archive(cereal::make_nvp("ToHit",m_ToHit));
    for(int i=0; i<24; ++i) {
        sprintf(buf,"DefenseType00%02d",i);
        archive(cereal::make_nvp(buf,m_DefenseTypes[i]));
    }

    archive(cereal::make_nvp("Defense",m_Defense));
    archive(cereal::make_nvp("Evade",m_Evade));
    archive(cereal::make_nvp("SpeedRunning",m_SpeedRunning));
    archive(cereal::make_nvp("SpeedFlying",m_SpeedFlying));
    archive(cereal::make_nvp("SpeedSwimming",m_SpeedSwimming));
    archive(cereal::make_nvp("SpeedJumping",m_SpeedJumping));
    archive(cereal::make_nvp("JumpHeight",m_jump_height));
    archive(cereal::make_nvp("MovementControl",m_MovementControl));
    archive(cereal::make_nvp("MovementFriction",m_MovementFriction));
    archive(cereal::make_nvp("Stealth",m_Stealth));
    archive(cereal::make_nvp("StealthRadius",m_StealthRadius));
    archive(cereal::make_nvp("PerceptionRadius",m_PerceptionRadius));
    archive(cereal::make_nvp("Regeneration",m_Regeneration));
    archive(cereal::make_nvp("Recovery",m_Recovery));
    archive(cereal::make_nvp("ThreatLevel",m_ThreatLevel));
    archive(cereal::make_nvp("Taunt",m_Taunt));
    archive(cereal::make_nvp("Confused",m_Confused));
    archive(cereal::make_nvp("Afraid",m_Afraid));
    archive(cereal::make_nvp("Held",m_Held));
    archive(cereal::make_nvp("Immobilized",m_Immobilized));
    archive(cereal::make_nvp("Stunned",m_is_stunned));
    archive(cereal::make_nvp("Sleep",m_Sleep));
    archive(cereal::make_nvp("Fly",m_is_flying));
    archive(cereal::make_nvp("Jumppack",m_has_jumppack));
    archive(cereal::make_nvp("Teleport",m_Teleport));
    archive(cereal::make_nvp("Untouchable",m_Untouchable));
    archive(cereal::make_nvp("Intangible",m_Intangible));
    archive(cereal::make_nvp("OnlyAffectsSelf",m_OnlyAffectsSelf));
    archive(cereal::make_nvp("Knockup",m_Knockup));
    archive(cereal::make_nvp("Knockback",m_Knockback));
    archive(cereal::make_nvp("Repel",m_Repel));
    archive(cereal::make_nvp("Accuracy",m_Accuracy));
    archive(cereal::make_nvp("Radius",m_Radius));
    archive(cereal::make_nvp("Arc",m_Arc));
    archive(cereal::make_nvp("Range",m_Range));
    archive(cereal::make_nvp("TimeToActivate",m_TimeToActivate));
    archive(cereal::make_nvp("RechargeTime",m_RechargeTime));
    archive(cereal::make_nvp("InterruptTime",m_InterruptTime));
    archive(cereal::make_nvp("EnduranceDiscount",m_EnduranceDiscount));
}
SPECIALIZE_SERIALIZATIONS(Parse_CharAttribMax)