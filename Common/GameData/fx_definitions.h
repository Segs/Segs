#pragma once
#include "Components/Colors.h"

#include "GameData/anim_definitions.h"
#include "GameData/seq_definitions.h"

#include <QtCore/QByteArray>
#include <vector>
#include <array>

struct TextureAnim_Data;

struct ColorFx
{
    RGBA startcolor;
    int startTime;
    float primaryTint; //i24
    float secondaryTint; //i24
};

enum SplatFlags
{
    SPLAT_ADDITIVE     = 1 << 0,
    SPLAT_SHADOWFADE   = 1 << 1,
    SPLAT_ADDBASE      = 1 << 2,
    SPLAT_ROUNDTEXTURE = 1 << 3,
    SPLAT_SUBTRACTIVE  = 1 << 4,
};

enum FxTransformFlags
{
    FxTransform_None          = 0,
    FxTransform_Position      = 1 << 0,
    FxTransform_Rotation      = 1 << 1,
    FxTransform_Scale         = 1 << 2,
    FxTransform_RotationExtra = 1 << 3,
    FxTransform_All           = FxTransform_Position | FxTransform_Rotation | FxTransform_Scale
};

enum PhysForceType : int
{
    PhysForceType_None,
    PhysForceType_Out,
    PhysForceType_In,
    PhysForceType_Up,
    PhysForceType_Forward,
    PhysForceType_Side,
    PhysForceType_CWSwirl,
    PhysForceType_CCWSwirl,
    PhysForceType_Drag,
};

enum PhysDebrisType : int
{
    PhysDebrisType_None,
    PhysDebrisType_Small,
    PhysDebrisType_Large,
    PhysDebrisType_LargeIfHardware,
};

enum PhysJointDOFType
{
    PhysJointDOFType_RotateX    = 1 << 0,
    PhysJointDOFType_RotateY    = 1 << 1,
    PhysJointDOFType_RotateZ    = 1 << 2,
    PhysJointDOFType_TranslateX = 1 << 3,
    PhysJointDOFType_TranslateY = 1 << 4,
    PhysJointDOFType_TranslateZ = 1 << 5,
};

enum SplatFalloffType : uint32_t;

struct FxBehavior
{
    std::vector<uint8_t> m_ParamBitfield; // bit set marking all overriding fields.
    QByteArray m_Name;
    uint32_t m_FileAge; // timestamp 32bit
    glm::vec3 m_StartJitter;
    glm::vec3 m_InitialVelocity;
    glm::vec3 m_InitialVelocityJitter;
    float m_InitialVelocityAngle = 0;
    float m_Gravity = 0;
    uint8_t m_Physics;
    float m_PhysRadius = 0;
    float m_PhysGravity = 1.0f;
    float m_PhysRestitution = 0;
    float m_PhysSFriction = 0;
    float m_PhysDFriction = 0;
    float m_PhysKillBelowSpeed = 0.0f;
    float m_PhysDensity = 1.0f;
    float m_PhysForceRadius = 0.0f;
    float m_PhysForcePower = 0.0f;
    float m_PhysForcePowerJitter = 0.0f;
    float m_PhysForceCentripetal = 2.0f;
    float m_PhysForceSpeedScaleMax = 0.0f;
    glm::vec3 m_PhysScale;

    PhysJointDOFType m_PhysJointDOFs = (PhysJointDOFType)0;
    glm::vec3 m_PhysJointAnchor;
    float m_PhysJointAngLimit = 0;
    float m_PhysJointLinLimit = 0;
    float m_PhysJointBreakTorque = 0.0f;
    float m_PhysJointBreakForce = 0.0f;
    float m_PhysJointLinSpring = 0;
    float m_PhysJointLinSpringDamp = 0;
    float m_PhysJointAngSpring = 0.0f;
    float m_PhysJointAngSpringDamp = 0.0f;
    float m_PhysJointDrag = 0;
    uint8_t m_PhysJointCollidesWorld;
    PhysForceType m_PhysForceType = PhysForceType_None;
    PhysDebrisType m_PhysDebris = PhysDebrisType_None;

    glm::vec3 m_Spin;
    glm::vec3 m_SpinJitter;
    float m_FadeInLength = 0;
    float m_FadeOutLength = 0;
    float m_Shake = 0;
    float m_ShakeFallOff = 0;
    float m_ShakeRadius = 0;
    float m_Blur = 0;
    float m_BlurFallOff = 0;
    float m_BlurRadius = 0;
    glm::vec3 m_Scale;
    glm::vec3 m_ScaleRate;
    glm::vec3 m_ScaleTime;
    glm::vec3 m_EndScale;
    uint8_t m_Stretch;
    float m_Drag = 0;
    glm::vec3 m_PyrRotate;
    glm::vec3 m_PyrRotateJitter;
    glm::vec3 m_PositionOffset;
    uint8_t m_UseShieldOffset; // bool
    float m_TrackRate = 0;
    uint8_t m_TrackMethod;
    uint8_t m_Collides;
    int m_LifeSpan;
    float m_AnimScale = 0;
    uint8_t m_Alpha;
    float m_PulsePeakTime = 0;
    float m_PulseBrightness = 0;
    float m_PulseClamp = 0;
    SplatFlags m_SplatFlags = SplatFlags(0);
    SplatFalloffType m_SplatFalloffType=SplatFalloffType(0);
    float m_SplatNormalFade = 0;
    float m_SplatFadeCenter = 0;
    float m_SplatSetBack = 0;
    std::vector<TextureAnim_Data> m_StAnim;
    float m_HueShift = 0;
    float m_HueShiftJitter = 0;
    std::array<ColorFx, 5> m_ColorEffect;
    uint8_t m_InheritGroupTint;
    uint8_t m_TintGeom; // bool

    bool initialized=false;
};

using Fx_AllBehaviors = std::vector<FxBehavior>;
struct EventSplat
{
    QByteArray tex1;
    QByteArray tex2;
};
enum GeoEvent_Flags {
    Event_AdoptParentEntity = 1 << 0,
    Event_NoReticle = 1<<1,
    Event_PowerLoopingSound = 1<<2,
    Event_OneAtATime = 1<<3,
//    Event_LookAtCamera = 2, no longer available in I24
};
struct FxSoundData
{
    QByteArray m_Name;
    float m_Radius = 100.0f;
    float m_Fade = 20.0f;
    float m_Volume = 1.0f;
};

enum class FxCollisionRotation : uint32_t
{
    UseCollisionNormal,
    UseWorldUp
};

struct FxGeoEntry_Event
{
    QByteArray m_EName;
    int m_Type;
    FxTransformFlags m_Inherit; // FxTransformFlag
    FxTransformFlags m_Update; // FxTransformFlag
    QByteArray m_At;
    QByteArray m_Bhvr;
    std::vector<FxBehavior> m_BhvrOverride;
    QByteArray m_JEvent;
    QByteArray m_CEvent;
    uint8_t m_CDestroy;
    uint8_t m_JDestroy;
    FxCollisionRotation m_CRotation;
    float m_ParentVelocityFraction;
    float m_CThresh;
    uint8_t m_HardwareOnly; //bool value
    uint8_t m_SoftwareOnly; //bool value
    uint8_t m_PhysicsOnly; //bool value
    uint8_t m_CameraSpace; //bool value
    float m_RayLength;
    QByteArray m_AtRayFx;
    std::vector<std::vector<QByteArray>> m_Geom;
    std::vector<std::vector<QByteArray>> m_Cape;
    int m_AltPiv;
    QByteArray m_AnimPiv;
    std::vector<std::vector<QByteArray>> m_Part;
    QByteArray m_Anim;
    QByteArray m_SetState;
    QByteArray m_ChildFx;
    QByteArray m_Magnet;
    QByteArray m_LookAt;
    QByteArray m_PMagnet;
    QByteArray m_POther;
    std::vector<EventSplat> m_Splats;
    std::vector<FxSoundData> m_Sounds;
    int m_SoundNoRepeat;
    float m_LifeSpan;
    float m_LifeSpanJitter;
    std::pair<uint8_t,uint8_t> m_Power;
    std::vector<std::vector<QByteArray>> m_While;
    std::vector<std::vector<QByteArray>> m_Until;
    QByteArray m_WorldGroup;
    GeoEvent_Flags m_Flags=(GeoEvent_Flags)0;


    int m_Debris=0;
    // Converted bits.
    SeqBitSet m_UntilBits;
    SeqBitSet m_WhileBits;
};
enum FxCondition_Bits
{
    CondFlag_None           = 0x0,
    CondFlag_Time           = 0x2,
    CondFlag_Primedied      = 0x4,
    CondFlag_Primehit       = 0x8,
    CondFlag_Primebeamhit   = 0x10,
    CondFlag_Cycle          = 0x20,
    CondFlag_Prime1hit      = 0x40,
    CondFlag_Prime2hit      = 0x80,
    CondFlag_Prime3hit      = 0x100,
    CondFlag_Prime4hit      = 0x200,
    CondFlag_Prime5hit      = 0x400,
    CondFlag_Triggerbits    = 0x800,
    CondFlag_Death          = 0x1000,
    CondFlag_DefaultSurface = 0x2000,
    CondFlag_Prime6hit      = 0x4000,
    CondFlag_Prime7hit      = 0x8000,
    CondFlag_Prime8hit      = 0x10000,
    CondFlag_Prime9hit      = 0x20000,
    CondFlag_Prime10hit     = 0x40000,

    CondFlag_AllPrimeHits = CondFlag_Prime1hit | CondFlag_Prime2hit | CondFlag_Prime3hit |
                            CondFlag_Prime4hit | CondFlag_Prime5hit | CondFlag_Primebeamhit | CondFlag_Primehit
};
struct FxInfo_Condition
{
    QByteArray m_On;
    float m_Time;
    float m_DayStart; // i24
    float m_DayEnd; // i24
    int m_Dist;
    float m_Chance;
    uint8_t DoMany;
    uint8_t m_Repeat;
    uint8_t m_RepeatJitter;
    std::vector<FxGeoEntry_Event> m_Event;
    uint8_t m_Random;
    float m_ForceThreshold;

    int trigger; // FxCondition_Bits
    std::vector<std::vector<QByteArray>> str_TriggerBits;
    SeqBitSet triggerstates[9];
};

struct FxInfo_Input
{
    QByteArray m_InpName;
};

enum FxInfo_Flags
{
    FxInfo_SoundOnly                   = 1 << 0,
    FxInfo_InheritAlpha                = 1 << 1,
    FxInfo_IsArmor                     = 1 << 2,
    FxInfo_InheritAnimScale            = 1 << 3,
    FxInfo_DontInheritBits             = 1 << 4,
    FxInfo_DontSuppress                = 1 << 5,
    FxInfo_DontInheritTexFromCostume   = 1 << 6,
    FxInfo_UseSkinTint                 = 1 << 7,
    FxInfo_IsShield                    = 1 << 8,
    FxInfo_IsWeapon                    = 1 << 9,
    FxInfo_NotCompatibleWithAnimalHead = 1 << 10,
    FxInfo_InheritGeoScale             = 1 << 11,
    FxInfo_UseTargetSeq                = 1 << 12
};

struct FxInfo
{
    std::vector<QByteArray> inputs;
    std::vector<FxInfo_Input> m_Inputs;
    std::vector<FxInfo_Condition> m_Conditions;
    QByteArray m_Name;
    uint32_t m_FileAge; // timestamp_t 32bit
    int m_LifeSpan;
    int m_Lighting;
    FxInfo_Flags m_Flags; //FxInfo_Flags
    float m_PerformanceRadius = 0.0f;
    float m_OnForceRadius = 0.0f;
    float m_AnimScale = 1.0f;
    glm::vec3 m_ClampMinScale;
    glm::vec3 m_ClampMaxScale;
    int hasEventsOnDeath;
    int m_initialized;
    uint8_t initialized() const { return m_initialized &0xFF;}
    int last_change_date;
};
using Fx_AllInfos = std::vector<FxInfo>;
