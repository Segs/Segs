/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <vector>
#include <array>
#include <QtCore/QString>

struct Parse_AttribDesc
{
    QString Name;
    QString DisplayName;
    QString IconName;
};

struct AttribNames_Data
{
    std::vector<Parse_AttribDesc> m_Damage;
    std::vector<Parse_AttribDesc> m_Defense;
    std::vector<Parse_AttribDesc> m_Boost;
    std::vector<Parse_AttribDesc> m_Group;
};

struct Parse_CharAttrib
{
    std::array<float,24> m_DamageTypes;
    float m_HitPoints;
    float m_Endurance;
    float m_ToHit;
    std::array<float,24> m_DefenseTypes;
    float m_Defense;
    float m_Evade;
    float m_SpeedRunning;
    float m_SpeedFlying;
    float m_SpeedSwimming;
    float m_SpeedJumping;
    float m_jump_height;
    float m_MovementControl;
    float m_MovementFriction;
    float m_Stealth;
    float m_StealthRadius;
    float m_PerceptionRadius;
    float m_Regeneration;
    float m_Recovery;
    float m_ThreatLevel;
    float m_Taunt;
    float m_Confused;
    float m_Afraid;
    float m_Held;
    float m_Immobilized;
    float m_is_stunned;
    float m_Sleep;
    float m_is_flying;
    float m_has_jumppack;
    float m_Teleport;
    float m_Untouchable;
    float m_Intangible;
    float m_OnlyAffectsSelf;
    float m_Knockup;
    float m_Knockback;
    float m_Repel;
    float m_Accuracy;
    float m_Radius;
    float m_Arc;
    float m_Range;
    float m_TimeToActivate;
    float m_RechargeTime;
    float m_InterruptTime;
    float m_EnduranceDiscount;
    float *begin() { return &m_DamageTypes[0]; }
    float *end() { return (&m_EnduranceDiscount)+1; }
    const float *begin() const { return &m_DamageTypes[0]; }
    const float *end() const { return (&m_EnduranceDiscount)+1; }
};

struct Parse_CharAttribMax
{
    std::array<std::vector<float>,24> m_DamageTypes;
    std::vector<float> m_HitPoints;
    std::vector<float> m_Endurance;
    std::vector<float> m_ToHit;
    std::array<std::vector<float>,24> m_DefenseTypes;
    std::vector<float> m_Defense;
    std::vector<float> m_Evade;
    std::vector<float> m_SpeedRunning;
    std::vector<float> m_SpeedFlying;
    std::vector<float> m_SpeedSwimming;
    std::vector<float> m_SpeedJumping;
    std::vector<float> m_jump_height;
    std::vector<float> m_MovementControl;
    std::vector<float> m_MovementFriction;
    std::vector<float> m_Stealth;
    std::vector<float> m_StealthRadius;
    std::vector<float> m_PerceptionRadius;
    std::vector<float> m_Regeneration;
    std::vector<float> m_Recovery;
    std::vector<float> m_ThreatLevel;
    std::vector<float> m_Taunt;
    std::vector<float> m_Confused;
    std::vector<float> m_Afraid;
    std::vector<float> m_Held;
    std::vector<float> m_Immobilized;
    std::vector<float> m_is_stunned;
    std::vector<float> m_Sleep;
    std::vector<float> m_is_flying;
    std::vector<float> m_has_jumppack;
    std::vector<float> m_Teleport;
    std::vector<float> m_Untouchable;
    std::vector<float> m_Intangible;
    std::vector<float> m_OnlyAffectsSelf;
    std::vector<float> m_Knockup;
    std::vector<float> m_Knockback;
    std::vector<float> m_Repel;
    std::vector<float> m_Accuracy;
    std::vector<float> m_Radius;
    std::vector<float> m_Arc;
    std::vector<float> m_Range;
    std::vector<float> m_TimeToActivate;
    std::vector<float> m_RechargeTime;
    std::vector<float> m_InterruptTime;
    std::vector<float> m_EnduranceDiscount;
};
