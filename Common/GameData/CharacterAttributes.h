/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <vector>
#include <array>
#include <QtCore/QString>

struct Parse_AttribDesc
{
    QByteArray Name;
    QByteArray DisplayName;
    QByteArray IconName;
    template<class Archive>
    void serialize(Archive & archive);
};

struct AttribNames_Data
{
    std::vector<Parse_AttribDesc> m_Damage;
    std::vector<Parse_AttribDesc> m_Defense;
    std::vector<Parse_AttribDesc> m_Boost;
    std::vector<Parse_AttribDesc> m_Group;
    std::vector<Parse_AttribDesc> m_Mode;
    std::vector<Parse_AttribDesc> m_Elusivity;
    std::vector<Parse_AttribDesc> m_StackKeys;
    template<class Archive>
    void serialize(Archive & archive);
};

struct Parse_CharAttrib
{
    //TODO: if we switch to I24 exlusively, this can be reduce to array of size 20
    std::array<float,24> m_DamageTypes;
    float m_HitPoints           = 0;
    float m_Absorb              = 0; //i24
    float m_Endurance           = 0;
    float m_Insight             = 0; //i24
    float m_Rage                = 0; //i24
    float m_ToHit               = 0;
    //TODO: if we switch to I24 exlusively, this can be reduce to array of size 20
    std::array<float,24> m_DefenseTypes;
    float m_Defense             = 0;
    float m_SpeedRunning        = 0;
    float m_SpeedFlying         = 0;
    float m_SpeedSwimming       = 0;
    float m_SpeedJumping        = 0;
    float m_JumpHeight         = 0;

    float m_MovementControl     = 0;
    float m_MovementFriction    = 0;
    float m_Stealth             = 0;
    float m_StealthRadius       = 0;
    float m_StealthRadiusPlayer = 0;
    float m_PerceptionRadius    = 0;
    float m_Regeneration        = 0;
    float m_Recovery            = 0;
    float m_InsightRecovery     = 0;
    float m_ThreatLevel         = 0;
    float m_Taunt               = 0;
    float m_Placate             = 0;
    float m_Confused            = 0;
    float m_Afraid              = 0;
    float m_Terrorized          = 0;
    float m_Held                = 0;
    float m_Immobilized         = 0;
    float m_IsStunned          = 0;
    float m_Sleep               = 0;

    float m_IsFlying           = 0;
    float m_HasJumppack        = 0;
    float m_Teleport            = 0;
    float m_Untouchable         = 0;
    float m_Intangible          = 0;
    float m_OnlyAffectsSelf     = 0;
    float m_ExperienceGain = 0.0f;
    float m_InfluenceGain = 0.0f;
    float m_PrestigeGain = 0.0f;
    float m_Evade               = 0; // not used in the client ? likely not used anywhere since elusivity came along.
    float m_Knockup             = 0;
    float m_Knockback           = 0;

    float m_Repel               = 0;
    float m_Accuracy            = 0;
    float m_Radius              = 0;
    float m_Arc                 = 0;
    float m_Range               = 0;
    float m_TimeToActivate      = 0;
    float m_RechargeTime        = 0;
    float m_InterruptTime       = 0;
    float m_EnduranceDiscount   = 0;
    float m_InsightDiscount     = 0;
    float m_Meter               = 0;
    std::array<float,20> m_ElusivityTypes;
    float m_ElusivityBase = 0;

    float *begin() { return &m_DamageTypes[0]; }
    float *end() { return (&m_ElusivityBase)+1; }
    const float *begin() const { return &m_DamageTypes[0]; }
    const float *end() const { return (&m_ElusivityBase)+1; }
    void initAttribArrays()
    {
        m_DamageTypes.fill(0.0f);
        m_DefenseTypes.fill(0.0f);
        m_ElusivityTypes.fill(0.0f);
    }
    template<class Archive>
    void serialize(Archive & archive);
};

struct Parse_CharAttribMax
{

    std::array<std::vector<float>,24> m_DamageTypes;
    std::vector<float > m_HitPoints;
    std::vector<float > m_Absorb;
    std::vector<float > m_Endurance;
    std::vector<float > m_Insight;
    std::vector<float > m_Rage;
    std::vector<float > m_ToHit;
    //TODO: if we switch to I24 exlusively, this can be reduce to array of size 20
    std::array<std::vector<float>,24> m_DefenseTypes;
    std::vector<float > m_Defense;
    std::vector<float > m_SpeedRunning;
    std::vector<float > m_SpeedFlying;
    std::vector<float > m_SpeedSwimming;
    std::vector<float > m_SpeedJumping;
    std::vector<float > m_JumpHeight;

    std::vector<float > m_MovementControl;
    std::vector<float > m_MovementFriction;
    std::vector<float > m_Stealth;
    std::vector<float > m_StealthRadius;
    std::vector<float > m_StealthRadiusPlayer;
    std::vector<float > m_PerceptionRadius;
    std::vector<float > m_Regeneration;
    std::vector<float > m_Recovery;
    std::vector<float > m_InsightRecovery;
    std::vector<float > m_ThreatLevel;
    std::vector<float > m_Taunt;
    std::vector<float > m_Placate;
    std::vector<float > m_Confused;
    std::vector<float > m_Afraid;
    std::vector<float > m_Terrorized;
    std::vector<float > m_Held;
    std::vector<float > m_Immobilized;
    std::vector<float > m_IsStunned;
    std::vector<float > m_Sleep;

    std::vector<float > m_IsFlying;
    std::vector<float > m_HasJumppack;
    std::vector<float > m_Teleport;
    std::vector<float > m_Untouchable;
    std::vector<float > m_Intangible;
    std::vector<float > m_OnlyAffectsSelf;
    std::vector<float > m_ExperienceGain;
    std::vector<float > m_InfluenceGain;
    std::vector<float > m_PrestigeGain;
    std::vector<float > m_Evade;
    std::vector<float > m_Knockup;
    std::vector<float > m_Knockback;

    std::vector<float > m_Repel;
    std::vector<float > m_Accuracy;
    std::vector<float > m_Radius;
    std::vector<float > m_Arc;
    std::vector<float > m_Range;
    std::vector<float > m_TimeToActivate;
    std::vector<float > m_RechargeTime;
    std::vector<float > m_InterruptTime;
    std::vector<float > m_EnduranceDiscount;
    std::vector<float > m_InsightDiscount;
    std::vector<float > m_Meter;
    std::array<std::vector<float>,20> m_ElusivityTypes;
    std::vector<float > m_ElusivityBase;

    template<class Archive>
    void serialize(Archive & archive);
};
