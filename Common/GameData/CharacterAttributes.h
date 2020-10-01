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
    template<class Archive>
    void serialize(Archive & archive);
};

struct Parse_CharAttrib
{
    std::array<float,24> m_DamageTypes;
    float m_HitPoints           = 0;
    float m_Endurance           = 0;
    float m_ToHit               = 0;
    std::array<float,24> m_DefenseTypes;
    float m_Defense             = 0;
    float m_Evade               = 0;
    float m_SpeedRunning        = 0;
    float m_SpeedFlying         = 0;
    float m_SpeedSwimming       = 0;
    float m_SpeedJumping        = 0;
    float m_jump_height         = 0;
    float m_MovementControl     = 0;
    float m_MovementFriction    = 0;
    float m_Stealth             = 0;
    float m_StealthRadius       = 0;
    float m_PerceptionRadius    = 0;
    float m_Regeneration        = 0;
    float m_Recovery            = 0;
    float m_ThreatLevel         = 0;
    float m_Taunt               = 0;
    float m_Confused            = 0;
    float m_Afraid              = 0;
    float m_Held                = 0;
    float m_Immobilized         = 0;
    float m_is_stunned          = 0;
    float m_Sleep               = 0;
    float m_is_flying           = 0;
    float m_has_jumppack        = 0;
    float m_Teleport            = 0;
    float m_Untouchable         = 0;
    float m_Intangible          = 0;
    float m_OnlyAffectsSelf     = 0;
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
    float *begin() { return &m_DamageTypes[0]; }
    float *end() { return (&m_EnduranceDiscount)+1; }
    const float *begin() const { return &m_DamageTypes[0]; }
    const float *end() const { return (&m_EnduranceDiscount)+1; }
    void initAttribArrays()
    {
        m_DamageTypes.fill(0.0f);
        m_DefenseTypes.fill(0.0f);
    }
    template<class Archive>
    void serialize(Archive & archive);
};

struct Parse_CharAttribMax
{
    std::array<std::vector<float>,24> m_DamageTypes ;
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
    template<class Archive>
    void serialize(Archive & archive);
};
