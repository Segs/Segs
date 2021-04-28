/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <bitset>
#include <stdint.h>
#include <unordered_map>

#include <QMap>
#include <qobjectdefs.h>

// Qt 5.14 has built-in specialization for std hash on some of their types
#ifndef QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF
namespace std
{
template <> struct hash<QByteArray>
{
    size_t operator()(const QByteArray &x) const { return qHash(x); }
};
} // namespace std
#endif

// WHY, WINDOWS, WHY ?
#ifdef FAR
#undef FAR
#endif
#ifdef NEAR
#undef NEAR
#endif
namespace SEGS
{
struct SequencerInstanceStorage;
struct SceneTreeNode;
enum class CollisionType;
enum class SelectionMode;
struct AnimTrack;
}
namespace SEGS_Enums
{
Q_NAMESPACE
enum class SeqBitNames : uint32_t
{
    INVALID_BIT           = 0,
    FORWARD               = 1,
    BACKWARD              = 2,
    STEPLEFT              = 3,
    STEPRIGHT             = 4,
    JUMP                  = 5,
    AIR                   = 6,
    APEX                  = 7,
    FLY                   = 8,
    BIGFALL               = 9,
    SWIM                  = 10,
    PROFILE               = 11,
    IDLE                  = 12,
    TURNRIGHT             = 13,
    TURNLEFT              = 14,
    COMBAT                = 15,
    WEAPON                = 16,
    GUN                   = 17,
    KUNGFU                = 18,
    CLUB                  = 19,
    ICESLIDE              = 20,
    CARRY                 = 21,
    SNEAK                 = 22,
    ARMCHAIR              = 23,
    BARSTOOL              = 24,
    SIT                   = 25,
    CLAW                  = 26,
    STUN                  = 27,
    SLEEP                 = 28,
    SWEEP                 = 29,
    LAUNCH                = 30,
    PREMODE               = 31,
    GRAVEL                = 32,
    ROCK                  = 33,
    TILE                  = 34,
    LAVA                  = 35,
    GRASS                 = 36,
    CONCRETE              = 37,
    MUD                   = 38,
    DIRT                  = 39,
    PUDDLE                = 40,
    METAL                 = 41,
    WOOD                  = 42,
    INDEEP                = 43,
    INWATER               = 44,
    INLAVA                = 45,
    PREDICTABLE           = 46,
    NOCOLLISION           = 47,
    WALKTHROUGH           = 48,
    CANTMOVE              = 49,
    HIDE                  = 50,
    DIENOW                = 51,
    TARGET                = 52,
    JUSTFIDDLESTICKINDOIT = 53, // fixed the original CoH name :)
    VARIATIONA            = 54,
    VARIATIONB            = 55,
    VARIATIONC            = 56,
    FAR                   = 57,
    CLOSE                 = 58,
    VERYCLOSE             = 59,
    WALK                  = 60,
    RUN                   = 61,
    FRONT                 = 62,
    BACK                  = 63,
    RIGHT                 = 64,
    LEFT                  = 65,
    HIGH                  = 66,
    MID                   = 67,
    LOW                   = 68,
    QUICK                 = 69,
    STRONG                = 70,
    FIERCE                = 71,
    HIT                   = 72,
    DEATH                 = 73,
    PUNCH                 = 74,
    KICK                  = 75,
    HAND                  = 76,
    SWORD                 = 77,
    SHIELD                = 78,
    HEAD                  = 79,
    FEET                  = 80,
    FLAME                 = 81,
    FIRE                  = 82,
    ENERGY                = 83,
    ICE                   = 84,
    WINGS                 = 85,
    JETPACK               = 86,
    HOVER                 = 87,
    EYE                   = 88,
    CHEST                 = 89,
    BACKPACK              = 90,
    ALTREADY              = 91,
    TWO_HAND              = 92,
    ATTACK                = 93,
    RADIUS                = 94,
    ELEMENT               = 95,
    MAGIC                 = 96,
    KNOCKBACK             = 97,
    BLOCK                 = 98,
    LIMP                  = 99,
    BLADE                 = 100,
    BLUNT                 = 101,
    HANDGUN               = 102,
    KNOCKDOWN             = 103,
    STARTOFFENSESTANCE    = 104,
    STARTDEFENSESTANCE    = 105,
    TEST0                 = 106,
    TEST1                 = 107,
    TEST2                 = 108,
    TEST3                 = 109,
    TEST4                 = 110,
    TEST5                 = 111,
    TEST6                 = 112,
    TEST7                 = 113,
    TEST8                 = 114,
    TEST9                 = 115,
    COMBO2                = 116,
    COMBO3                = 117,
    SPEED                 = 118,
    SPELLCAST             = 119,
    RUMBLEA               = 120,
    RUMBLEB               = 121,
    PROP                  = 122,
    UNYIELD               = 123,
    CHOKESTUN             = 124,
    SONICSTUN             = 125,
    BLINDSTUN             = 126,
    LASSOSTUN             = 127,
    ELECTROSTUN           = 128,
    WEPHIP                = 129,
    WEPBACK               = 130,
    WEPUNDERARM           = 131,
    WEPLEG                = 132,
    SPAWNHIP              = 133,
    SPAWNBACK             = 134,
    SPAWNSIDE             = 135,
    SPAWNSHOULDER         = 136,
    SPAWNELEMENT          = 137,
    SNIPERBLAST           = 138,
    DUAL                  = 139,
    REV                   = 140,
    SPIN                  = 141,
    SWIPE                 = 142,
    SLASH                 = 143,
    CHOP                  = 144,
    STAB                  = 145,
    UPPERCUT              = 146,
    PARRY                 = 147,
    EMERGE                = 148,
    LEADERSHIP            = 149,
    SPEECH                = 150,
    MEDSELF               = 151,
    MEDOTHER              = 152,
    MEDDEAD               = 153,
    GETUP                 = 154,
    TRAIN                 = 155,
    REPAIR                = 156,
    RAMPAGE               = 157,
    VANDAL                = 158,
    COWER                 = 159,
    MACE                  = 160,
    AXE                   = 161,
    SIA                   = 162,
    SHURIKEN              = 163,
    KATANA                = 164,
    STAFF                 = 165,
    SHOTGUN               = 166,
    MACHINEGUN            = 167,
    RIFLE                 = 168,
    FLOATSTUN             = 169,
    SHOOT                 = 170,
    CONE                  = 171,
    BLAST                 = 172,
    BOX                   = 173,
    POSE                  = 174,
    GREETING              = 175,
    EXCLAMATION           = 176,
    CAST                  = 201,
    SUMMON                = 202,
    SPAWN                 = 203,
    KNIFE                 = 204,
    WAVE                  = 205,
    BRAWL                 = 206,
    SHUT                  = 207,
    OPEN                  = 208,
    LEVITATE              = 209,
    FORCE                 = 210,
    TELEKINESIS           = 211,
    PSIONIC               = 212,
    WALL                  = 213,
    CAGE                  = 214,
    BOOST                 = 215,
    BOOSTUP               = 216,
    BOOSTDOWN             = 217,
    TELEPORT              = 218,
    STEAL                 = 219,
    HEAL                  = 220,
    WRIGGLE               = 221,
    LEAPKICK              = 222,
    SPEEDPUNCH            = 223,
    TELEATTACK            = 224,
    HEADCAST              = 225,
    PSIONICBLAST          = 226,
    RADIUSEFFECT          = 227,
    RADIUSATTACK          = 228,
    GROUNDSUMMON          = 229,
    SKYSUMMON             = 230,
    WEPSPAWN              = 231,
    WEPSPAWN2             = 232,
    WEPSPAWN3             = 233,
    POWERPUNCH            = 234,
    SHAKE                 = 235,
    DRAIN                 = 236,
    CHOKE                 = 237,
    HEADPAIN              = 238,
    FEAR                  = 239,
    BOW                   = 240,
    CROSSBOW              = 241,
    THROWKNIFE            = 242,
    THROWSTAR             = 243,
    GRENADE               = 244,
    CHAINGUN              = 245,
    BOOMERANG             = 246,
    BALLCAST              = 247,
    SMASH                 = 248,
    DISRUPT               = 249,
    KUNGFUPOWER           = 250,
    KUNGFUSPEED           = 251,
    KUNGFUAREA            = 252,
    KUNGFUSTUN            = 253,
    MAGICTHROW            = 254,
    STONEARMOR            = 255,
    STOMP                 = 256,
    RAISEDEAD             = 257,
    FLAMETHROWER          = 258,
    ROCKETLAUNCHER        = 259,
    WINDBLAST             = 260,
    BLINDING              = 261,
    MEGABLAST             = 262,
    BLOW                  = 263,
    FIREAURA              = 264,
    RAPIDBLAST            = 265,
    POISON                = 266,
    SUPERBOOST            = 267,
    CLAWSPEED             = 268,
    CLAWAREA              = 269,
    LOOT                  = 270,
    MENACE                = 271,
    THREAT                = 272,
    LOITER                = 273,
    PUSH                  = 274,
    PURSETUG              = 275,
    GROUNDPUNCH           = 276,
    VANDALIZE             = 277,
    RUNOUT                = 278,
    SPRAYCAN              = 279,
    THANK                 = 280,
    BOOSTAURA             = 281,
    LOOKOUT               = 282,
    CHEER                 = 283,
    DEALING               = 284,
    RECEIVING             = 285,
    OBSERVE               = 286,
    TALK                  = 287,
    LISTEN                = 288,
    ROLLDICE              = 289,
    CHANTING              = 290,
    BOWING                = 291,
    MEDITATE              = 292,
    MOMENTGLORY           = 293,
    GUTPUNCH              = 294,
    JUMPNSMASH            = 295,
    BASEBALLPUNCH         = 296,
    SHORTTOSS             = 297,
    WINDUPPERCUT          = 298,
    TAUNTA                = 299,
    TAUNTB                = 200,
    TAUNTC                = 301,
    STOPCAST              = 302,
    SCREAM                = 303,
    TERRIFY               = 304,
    ENERGYTORRENT         = 305,
    HYPNOTIZE             = 306,
    JUMPCLAW              = 307,
    WINDUP                = 308,
    MAGICWINDUP           = 309,
    POWERWINDUP           = 310,
    MYSICWINDUP           = 311,
    TECHWINDUP            = 312,
    POINT                 = 313,
    LAUGH                 = 314,
    SAD                   = 315,
    HAPPY                 = 316,
    ANGRY                 = 317,
    LOVE                  = 318,
    EVIL                  = 319,
    FLEX                  = 320,
    BECKON                = 321,
    CLAP                  = 322,
    RUDE                  = 323,
    YES                   = 324,
    NO                    = 325,
    SHRUG                 = 326,
    DANCE                 = 327,
    SALUTE                = 328,
    BUSY                  = 329,
    STOP                  = 330,
    WAIT                  = 331,
    HELP                  = 332,
    YELL                  = 333,
    CHAT                  = 334,
    COMMAND               = 335,
    JOBSDONE              = 336,
    PLAN                  = 337,
    WELDING               = 338,
    WORK                  = 339,
    BODY                  = 340,
    BOULDER               = 341,
    ZOMBIE                = 342,
    KNEEL                 = 343,
    PLAYER                = 177,
    RECLAIM               = 344,
    CLIPBOARD             = 345,
    BRIEFCASE             = 346,
    CELLPHONE             = 347,
    NEWSPAPER             = 348,
    EAT                   = 349,
    CARPET                = 178,
    INSEWERWATER          = 179,
    SLIDING               = 180,
    BOUNCING              = 181,
    WARP                  = 182,
    RUNIN                 = 183,
    GAME                  = 350,
    RPS                   = 351,
    DICE                  = 352,
    ARGUE                 = 353,
    REASON                = 354,
    QUESTION              = 355,
    AFK                   = 356,
    ELEVATOR              = 357,
    SCORE                 = 358,
    ONE                   = 359,
    TWO                   = 360,
    THREE                 = 361,
    FOUR                  = 362,
    FIVE                  = 363,
    SIX                   = 364,
    SEVEN                 = 365,
    EIGHT                 = 366,
    NINE                  = 367,
    TEN                   = 368,
    ZERO                  = 369,

    NON_EXISTING = ~0U
};
Q_ENUM_NS(SeqBitNames)
} // end of SEGS_Enums namespace
using namespace SEGS_Enums;

struct SeqBitSet
{
    // Older combinations of Qt/moc/CMake require Q_GADGET macro and granting public access
    // to members. Otherwise, moc files are not generated properly under certain conditions.
    Q_GADGET
public:
    std::bitset<416> bits;
    bool isSet(SeqBitNames v) const { return bits[uint32_t(v)]; }
    void set(SeqBitNames bit) { bits[uint32_t(bit)] = true;}
    void setVal(SeqBitNames bit, bool v) { bits[uint32_t(bit)] = v; }
};

struct SeqMoveDataTypeAnim
{
    QByteArray name;
    int firstFrame; // first frame ticks
    int lastFrame; // last frame tick ?
};

struct SeqPlayFxData
{
    QByteArray name;
    uint32_t delay;
    uint32_t flags;
};

struct SeqMoveTypeData
{
    QByteArray                         name;
    std::vector<SeqMoveDataTypeAnim>   m_Anim;
    std::vector<SeqPlayFxData>         m_PlayFx;
    float                              Scale;
    float                              MoveRate;
    float                              PitchAngle;
    float                              PitchRate;
    float                              PitchStart;
    float                              PitchEnd;
    float                              SmoothSprint;
    // Transient data
    HandleT<20,12,SEGS::AnimTrack>     m_anm_track;
};

struct SeqNextMoveData
{
    QByteArray name;
};

struct SeqCycleMoveData
{
    QByteArray name;
};

enum
{
    MAXMOVES = 2048
};

struct SeqTypeAnimation
{
    SeqMoveTypeData *m_template;
    HandleT<20,12,SEGS::AnimTrack> m_anm_track;
};
// Structure that holds all transient data of a SeqMove
struct SeqMoveRawData
{
    SeqBitSet requires_bits;
    std::vector<int16_t> interrupted_by;
    int16_t   nextMove[4];
    uint8_t   num_nextmoves;
    int16_t   cycleMove[4];
    uint8_t   cycleMoveCnt;
    SeqBitSet sets_bits;
    SeqBitSet sets_child_bits;
    SeqBitSet sticks_on_child_bits;
    const struct SeqMoveData *m_source_data;
    int idx;  // index of source move in SeqMoveData
    //std::vector<SeqTypeAnimation> m_type_animations;
};

struct SeqMoveData
{
    QByteArray name;
    float Scale;
    float MoveRate;
    uint32_t Interpolate;
    uint32_t Priority;
    uint32_t Flags;
    std::vector<SeqNextMoveData> m_NextMove;
    std::vector<SeqCycleMoveData> m_CycleMove;
    std::vector<SeqMoveTypeData> m_Type;
    std::vector<QByteArray > SticksOnChild;
    std::vector<QByteArray > SetsOnChild;
    std::vector<QByteArray > Sets;
    std::vector<QByteArray > Requires;
    std::vector<QByteArray > Member;
    std::vector<QByteArray > Interrupts;

    enum eFlags
    {
        Cycle           = 0x0001,
        GlobalAnim      = 0x0002,
        FinishCycle     = 0x0004,
        ReqInputs       = 0x0008,
        ComplexCycle    = 0x0010,
        NoInterp        = 0x0020,
        HitReact        = 0x0040,
        NoSizeScale     = 0x0080,
        MoveScale       = 0x0100,
        NotSelectable   = 0x0200,
        SmoothSprint    = 0x0400,
        PitchToTarget   = 0x0800,
        PickRandomly    = 0x1000,
        FullSizeScale   = 0x2000,
        AlwaysSizeScale = 0x4000
    };
    SeqMoveRawData m_raw;
};

struct SeqGroupNameData
{
    QByteArray name;
};

struct SeqTypeDefData
{
    QByteArray name;
    QByteArray pBaseSkeleton;
    QByteArray pParentType;
};

struct SequencerData
{
    QByteArray name;
    std::vector<SeqTypeDefData> m_TypeDef;
    std::vector<SeqGroupNameData> m_Group;
    std::vector<SeqMoveData> m_Move;
};

struct SequencerList
{
    /// @note the size of this std::vector *cannot* be modified after load, since seqGetMoveIdxByName returns pointers to it's members.
    /// @todo consider using Handle pattern here ?
    std::vector<SequencerData> sq_list;
    int dev_seqInfoCount;
    QMap<QString, int> m_Sequencers; // ordered by sequencer's name ( implemented by 'operator <' )
    SequencerData * getSequencerData(const QByteArray &seq_name);
};
// Binds an NPC/Player to a SequencerData
struct EntitySequencerData
{
    QByteArray m_name;
    QByteArray m_sequencer_name;
    QByteArray m_seq_type;
    QByteArray m_graphics;
    QByteArray m_lod_names[3];
    const QByteArray &lod_Name(int idx) const
    {
        switch(idx) {
        case 0:
            return m_graphics;
        case 1:
        case 2:
        case 3:
            return m_lod_names[idx-1];
        default:
            assert(false);
        }
        return m_graphics;
    }
    float m_lod_dists[4] = {0,0,0,0};
    float m_visibility_sphere_radius=0;
    int m_max_alpha=0;
    float m_reverse_fade_out_distance=0;
    float m_fade_out_start=0;
    float m_fade_out_finish=0;
    QByteArray m_shadow;
    int m_use_shadow=0;
    int m_ticks_to_linger_after_death=0;
    int m_ticks_to_fade_away_after_death=0;
    QByteArray m_shadow_type;
    QByteArray m_shadow_texture;
    QByteArray m_shadow_quality;
    QByteArray m_flags;
    glm::vec3 m_shadow_size = {0,0,0};
    glm::vec3 m_shadow_offset = {0,0,0};
    int m_light_as_door_outside; // switch over to bool ?
    glm::vec3 m_geometry_scale = {0,0,0};
    //! \note if this is set the resulting scale will be randomly selected between m_geometry_scale and this
    glm::vec3 m_geometry_scale_max = {0,0,0};
    float m_anim_scale=0;
    QByteArray m_effect_names[5];
    int m_has_random_name=0;
    glm::vec3 m_collision_size = {0,0,0};
    glm::vec3 m_collision_offset = {0,0,0};
    QByteArray m_world_group;
    float m_minimum_ambient=0;
    QByteArray m_bone_scale_fat;
    QByteArray m_bone_scale_skinny;
    int m_random_bone_scale=0;
    int m_not_selectable=0;
    int m_no_collision=0;
    float m_bounciness=0;
    QByteArray m_collision_type;
    QByteArray m_placement;
    QByteArray m_selection;
    QByteArray m_constant_state;

    QByteArray m_cape_file;
    QByteArray m_cape_name;
    QByteArray m_cape_harness_file;
    QByteArray m_cape_harness_name;
    QByteArray m_cape_textures[2];
    QByteArray m_cape_inner_textures[2];
    glm::vec3 m_cape_colors[2] = {{0,0,0},{0,0,0}};
    glm::vec3 m_inner_cape_colors[2] = {{0,0,0},{0,0,0}};
    glm::vec2 m_reticle_mod = {0,0}; // targeting reticle fixup translation

    // transient fields
    SEGS::CollisionType m_converted_collision_type=SEGS::CollisionType(0);
    SEGS::SelectionMode m_converted_selection_mode=SEGS::SelectionMode(0);
    int m_converted_shadow_type=0;
    int m_converted_shadow_quality=0;
    int m_converted_flags=0;
    int m_converted_placement=0;
    SeqBitSet m_converted_constant_bits;
};
void cleanSeqFileName(QByteArray &filename);
int16_t getSeqMoveIdxByName(const QByteArray &name, const SequencerData &seq);
using SequencerTypeMap = std::unordered_map<QByteArray,EntitySequencerData>;
