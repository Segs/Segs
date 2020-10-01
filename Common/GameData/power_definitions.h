/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QHash>
#include <vector>
#include <qobjectdefs.h>

namespace SEGS_Enums
{
enum class SeqBitNames : uint32_t;
}
using namespace SEGS_Enums;

namespace SEGS_Enums_Power
{
Q_NAMESPACE

enum class StoredPower_Type
{
    Click       = 0,
    Auto        = 1,
    Toggle      = 2,
    Boost       = 3,
    Inspiration = 4,
};

enum class StoredEntEnum : uint32_t
{
    None                = 0,
    Caster              = 1,
    Player              = 2,
    DeadPlayer          = 3,
    Teammate            = 4,
    Enemy               = 5,
    DeadVillain         = 6,
    NPC                 = 7,
    Friend              = 8,
    Foe                 = 9,
    Location            = 10,
    Any                 = 11,
    DeadTeammate        = 12,
    DeadOrAliveTeammate = 13,
    Teleport            = 14,
};

enum StoredVisibility : uint32_t
{
    LineOfSight    = 0,
    VisibilityNone = 1,
};

enum class StoredAffectArea : uint32_t
{
    Character = 0,
    Cone      = 1,
    Sphere    = 2,
    Location  = 3,
};

enum class StoredAiReport : uint32_t
{
    Always   = 0,
    Never    = 1,
    HitsOnly = 2,
    MissOnly = 3,
};

enum DurationEnum : int32_t
{
    kInstant     = -1,
    kUntilKilled = 999999, // Until Shut Off
};

enum class AttribStackType
{
    Stack   = 0,
    Ignore  = 1,
    Extend  = 2,
    Replace = 3
};

enum class AttribModType : uint32_t
{
    Duration  = 0,
    Magnitude = 1,
    Constant  = 2,
};

enum class AttribModTarget : uint32_t
{
    Self   = 0,
    Target = 1,
};

enum class AttribMod_Aspect : uint32_t
{
    Current         = 0,
    Maximum         = 4,
    Strength        = 8,
    Resistance      = 0xC,
    Absolute        = 0x10,
    CurrentAbsolute = 0x10,
};

enum class PowerType : uint32_t
{
    Click       = 0,
    Auto        = 1,
    Toggle      = 2,
    Boost       = 3,
    Inspiration = 4,
    NumTypes    = 5,
};

// I'm unsure as to the validity of this
enum class AttackType : uint32_t {
    None = 0,
    Claw = 1,
    Kunfu = 2,
    Gun = 3,
    Blade = 4,
    Blunt = 5,
    Handgun = 6,
    Combat = 7,
    Weapon = 8,
    Speed = 9,
    Teleport = 10,
    Carry = 11,
    Club = 12,
    Ranged = 108,
    Melee = 112,
    Aoe = 116,
    Smashing = 120,
    Lethal = 124,
    Fire = 128,
    Cold = 132,
    Energy = 136,
    Negative_Energy = 140
};
Q_ENUM_NS(AttackType)

} // end of SEGS_Enums_Power namespace
using namespace SEGS_Enums_Power;

struct StoredAttribMod
{
    // Older combinations of Qt/moc/CMake require Q_GADGET macro and granting public access
    // to members. Otherwise, moc files are not generated properly under certain conditions.
    Q_GADGET
public:
    QByteArray         name             = "unknown";
    int                index_in_power;
    QByteArray         DisplayAttackerHit = "";
    QByteArray         DisplayVictimHit = "";
    struct Power_Data *parent_StoredPower;
    AttribModTarget    Target           = AttribModTarget::Target;
    QByteArray         Table            = "43"; //would be easier as int, since the tables are stored in an array
    float              Scale            = 1.0;
    int                Attrib           = 0; //
    AttribMod_Aspect   Aspect;
    AttribModType      Type             = AttribModType::Magnitude;
    float              Duration         = 0.0; // Special values in DurationEnum
    float              Magnitude        = 1.0;
    int                Delay            = 0;    //should be float, for now I'm just dividing by 1000
    int                Period           = 0;    //same
    int                Chance           = 100;  // are we sure this isn't a float?
    int                CancelOnMiss;            // bool?
    int                NearGround;              // bool?
    int                AllowStrength;           // bool?
    int                AllowResistance;         // bool?
    AttribStackType    StackType     = AttribStackType::Replace;
    std::vector<int>   ContinuingBits;
    QByteArray         ContinuingFX;
    std::vector<int>   ConditionalBits; // 5c
    QByteArray         ConditionalFX;
    QByteArray         EntityDef;
    QByteArray         PriorityListOffense;
    QByteArray         PriorityListDefense;
    QByteArray         PriorityListPassive;
};

struct Power_Data
{
    QByteArray                   m_Name;
    int                          ptr_powerset_available;
    struct Parse_PowerSet *      parent_StoredPowerSet;
    int                          category_idx;
    int                          powerset_idx;
    int                          power_index;
    QByteArray                   DisplayName;
    QByteArray                   DisplayHelp;
    QByteArray                   DisplayShortHelp;
    QByteArray                   DisplayAttackerAttack;
    QByteArray                   DisplayAttackerHit;
    QByteArray                   DisplayVictimHit;
    QByteArray                   IconName;
    std::vector<SeqBitNames>     ModeSeqBits;
    std::vector<SeqBitNames>     ActivationBits;
    std::vector<SeqBitNames>     WindUpBits; // 3c
    std::vector<SeqBitNames>     InitialAttackBits;
    std::vector<SeqBitNames>     AttackBits;
    std::vector<SeqBitNames>     HitBits;
    std::vector<SeqBitNames>     BlockBits;
    std::vector<SeqBitNames>     DeathBits;
    QByteArray                   ActivationFX;
    QByteArray                   WindUpFX;
    QByteArray                   InitialAttackFX;
    QByteArray                   AttackFX;
    QByteArray                   BlockFX;
    QByteArray                   HitFX;
    QByteArray                   DeathFX;
    int                          m_InitialFramesBeforeHit;
    int                          m_FramesBeforeHit;
    int                          m_AttackFrames;
    int                          DelayedHit;
    int                          ProjectileSpeed;
    PowerType                    Type;
    std::vector<AttackType>      AttackTypes;
    std::vector<QByteArray>      Requires;
    float                        Accuracy;
    int                          IgnoreStrength;
    int                          NearGround;
    int                          TargetNearGround;
    int                          CastableAfterDeath;
    int                          AIReport;
    StoredAffectArea             EffectArea;
    float                        Radius;
    float                        Arc;
    float                        Range;
    float                        RangeSecondary;
    float                        InitialFramesBeforeHit_seconds;
    float                        FramesBeforeHit_seconds;
    float                        TimeToActivate;
    float                        RechargeTime;
    float                        InterruptTime;
    float                        ActivatePeriod;
    float                        EnduranceCost;
    int                          DestroyOnLimit;
    int                          limited_use;
    int                          m_NumCharges;
    float                        m_UsageTime;
    int                          has_lifetime;
    float                        m_Lifetime;
    StoredVisibility             TargetVisibility;
    StoredEntEnum                Target;
    StoredEntEnum                TargetSecondary;
    std::vector<StoredEntEnum>   EntsAffected;
    std::vector<StoredEntEnum>   EntsAutoHit;
    std::vector<uint32_t>        BoostsAllowed;
    std::vector<uint32_t>        GroupMembership;
    std::vector<QByteArray>      AIGroups;
    std::vector<StoredAttribMod> pAttribMod;
    int                          fDamageGiven;
    int                          iCntUsed;
    int                          iCntHits;
    int                          iCntMisses;
};

struct Parse_PowerSet
{
    QByteArray                   m_Name;
    struct StoredPowerCategory * parent_PowerCategory;
    QByteArray                   DisplayName;
    QByteArray                   DisplayHelp;
    QByteArray                   DisplayShortHelp;
    QByteArray                   IconName;
    std::vector<Power_Data>      m_Powers;
    QHash<QString, Power_Data *> m_hash_table;
    std::vector<int32_t>         Available;
};

enum
{
    kCategory_Count = 3
};

struct StoredPowerCategory
{
    QByteArray                       name;
    QByteArray                       disp_name;
    QByteArray                       disp_help;
    QByteArray                       disp_short_help;
    std::vector<Parse_PowerSet>      m_PowerSets;
    QHash<QString, Parse_PowerSet *> m_powers_hash;
};

struct AllPowerCategories
{
    std::vector<StoredPowerCategory>      m_categories;
    QHash<QString, StoredPowerCategory *> categories_hash;
};
