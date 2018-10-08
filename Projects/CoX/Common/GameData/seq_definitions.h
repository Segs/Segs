/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <vector>
#include <QMap>
#include <bitset>

#include <stdint.h>

struct AnimTrack;
// WHY, WINDOWS, WHY ?
#ifdef FAR
#undef FAR
#endif
#ifdef NEAR
#undef NEAR
#endif

enum class SeqBitNames : uint32_t
{
    SB_INVALID_BIT=0,
    SB_FORWARD=1,
    SB_BACKWARD=2,
    SB_STEPLEFT=3,
    SB_STEPRIGHT=4,
    SB_JUMP=5,
    SB_AIR=6,
    SB_APEX=7,
    SB_FLY=8,
    SB_BIGFALL=9,
    SB_SWIM=10,
    SB_PROFILE=11,
    SB_IDLE=12,
    SB_TURNRIGHT=13,
    SB_TURNLEFT=14,
    SB_COMBAT=15,
    SB_WEAPON=16,
    SB_GUN=17,
    SB_KUNGFU=18,
    SB_CLUB=19,
    SB_ICESLIDE=20,
    SB_CARRY=21,
    SB_SNEAK=22,
    SB_ARMCHAIR=23,
    SB_BARSTOOL=24,
    SB_SIT=25,
    SB_CLAW=26,
    SB_STUN=27,
    SB_SLEEP=28,
    SB_SWEEP=29,
    SB_LAUNCH=30,
    SB_PREMODE=31,
    SB_GRAVEL=32,
    SB_ROCK=33,
    SB_TILE=34,
    SB_LAVA=35,
    SB_GRASS=36,
    SB_CONCRETE=37,
    SB_MUD=38,
    SB_DIRT=39,
    SB_PUDDLE=40,
    SB_METAL=41,
    SB_WOOD=42,
    SB_INDEEP=43,
    SB_INWATER=44,
    SB_INLAVA=45,
    SB_PREDICTABLE=46,
    SB_NOCOLLISION=47,
    SB_WALKTHROUGH=48,
    SB_CANTMOVE=49,
    SB_HIDE=50,
    SB_DIENOW=51,
    SB_TARGET=52,
    SB_JUSTFUCKINDOIT=53,
    SB_VARIATIONA=54,
    SB_VARIATIONB=55,
    SB_VARIATIONC=56,
    SB_FAR=57,
    SB_CLOSE=58,
    SB_VERYCLOSE=59,
    SB_WALK=60,
    SB_RUN=61,
    SB_FRONT=62,
    SB_BACK=63,
    SB_RIGHT=64,
    SB_LEFT=65,
    SB_HIGH=66,
    SB_MID=67,
    SB_LOW=68,
    SB_QUICK=69,
    SB_STRONG=70,
    SB_FIERCE=71,
    SB_HIT=72,
    SB_DEATH=73,
    SB_PUNCH=74,
    SB_KICK=75,
    SB_HAND=76,
    SB_SWORD=77,
    SB_SHIELD=78,
    SB_HEAD=79,
    SB_FEET=80,
    SB_FLAME=81,
    SB_FIRE=82,
    SB_ENERGY=83,
    SB_ICE=84,
    SB_WINGS=85,
    SB_JETPACK=86,
    SB_HOVER=87,
    SB_EYE=88,
    SB_CHEST=89,
    SB_BACKPACK=90,
    SB_ALTREADY=91,
    SB_2HAND=92,
    SB_ATTACK=93,
    SB_RADIUS=94,
    SB_ELEMENT=95,
    SB_MAGIC=96,
    SB_KNOCKBACK=97,
    SB_BLOCK=98,
    SB_LIMP=99,
    SB_BLADE=100,
    SB_BLUNT=101,
    SB_HANDGUN=102,
    SB_KNOCKDOWN=103,
    SB_STARTOFFENSESTANCE=104,
    SB_STARTDEFENSESTANCE=105,
    SB_TEST0=106,
    SB_TEST1=107,
    SB_TEST2=108,
    SB_TEST3=109,
    SB_TEST4=110,
    SB_TEST5=111,
    SB_TEST6=112,
    SB_TEST7=113,
    SB_TEST8=114,
    SB_TEST9=115,
    SB_COMBO2=116,
    SB_COMBO3=117,
    SB_SPEED=118,
    SB_SPELLCAST=119,
    SB_RUMBLEA=120,
    SB_RUMBLEB=121,
    SB_PROP=122,
    SB_UNYIELD=123,
    SB_CHOKESTUN=124,
    SB_SONICSTUN=125,
    SB_BLINDSTUN=126,
    SB_LASSOSTUN=127,
    SB_ELECTROSTUN=128,
    SB_WEPHIP=129,
    SB_WEPBACK=130,
    SB_WEPUNDERARM=131,
    SB_WEPLEG=132,
    SB_SPAWNHIP=133,
    SB_SPAWNBACK=134,
    SB_SPAWNSIDE=135,
    SB_SPAWNSHOULDER=136,
    SB_SPAWNELEMENT=137,
    SB_SNIPERBLAST=138,
    SB_DUAL=139,
    SB_REV=140,
    SB_SPIN=141,
    SB_SWIPE=142,
    SB_SLASH=143,
    SB_CHOP=144,
    SB_STAB=145,
    SB_UPPERCUT=146,
    SB_PARRY=147,
    SB_EMERGE=148,
    SB_LEADERSHIP=149,
    SB_SPEECH=150,
    SB_MEDSELF=151,
    SB_MEDOTHER=152,
    SB_MEDDEAD=153,
    SB_GETUP=154,
    SB_TRAIN=155,
    SB_REPAIR=156,
    SB_RAMPAGE=157,
    SB_VANDAL=158,
    SB_COWER=159,
    SB_MACE=160,
    SB_AXE=161,
    SB_SIA=162,
    SB_SHURIKEN=163,
    SB_KATANA=164,
    SB_STAFF=165,
    SB_SHOTGUN=166,
    SB_MACHINEGUN=167,
    SB_RIFLE=168,
    SB_FLOATSTUN=169,
    SB_SHOOT=170,
    SB_CONE=171,
    SB_BLAST=172,
    SB_BOX=173,
    SB_POSE=174,
    SB_GREETING=175,
    SB_EXCLAMATION=176,
    SB_CAST=201,
    SB_SUMMON=202,
    SB_SPAWN=203,
    SB_KNIFE=204,
    SB_WAVE=205,
    SB_BRAWL=206,
    SB_SHUT=207,
    SB_OPEN=208,
    SB_LEVITATE=209,
    SB_FORCE=210,
    SB_TELEKINESIS=211,
    SB_PSIONIC=212,
    SB_WALL=213,
    SB_CAGE=214,
    SB_BOOST=215,
    SB_BOOSTUP=216,
    SB_BOOSTDOWN=217,
    SB_TELEPORT=218,
    SB_STEAL=219,
    SB_HEAL=220,
    SB_WRIGGLE=221,
    SB_LEAPKICK=222,
    SB_SPEEDPUNCH=223,
    SB_TELEATTACK=224,
    SB_HEADCAST=225,
    SB_PSIONICBLAST=226,
    SB_RADIUSEFFECT=227,
    SB_RADIUSATTACK=228,
    SB_GROUNDSUMMON=229,
    SB_SKYSUMMON=230,
    SB_WEPSPAWN=231,
    SB_WEPSPAWN2=232,
    SB_WEPSPAWN3=233,
    SB_POWERPUNCH=234,
    SB_SHAKE=235,
    SB_DRAIN=236,
    SB_CHOKE=237,
    SB_HEADPAIN=238,
    SB_FEAR=239,
    SB_BOW=240,
    SB_CROSSBOW=241,
    SB_THROWKNIFE=242,
    SB_THROWSTAR=243,
    SB_GRENADE=244,
    SB_CHAINGUN=245,
    SB_BOOMERANG=246,
    SB_BALLCAST=247,
    SB_SMASH=248,
    SB_DISRUPT=249,
    SB_KUNGFUPOWER=250,
    SB_KUNGFUSPEED=251,
    SB_KUNGFUAREA=252,
    SB_KUNGFUSTUN=253,
    SB_MAGICTHROW=254,
    SB_STONEARMOR=255,
    SB_STOMP=256,
    SB_RAISEDEAD=257,
    SB_FLAMETHROWER=258,
    SB_ROCKETLAUNCHER=259,
    SB_WINDBLAST=260,
    SB_BLINDING=261,
    SB_MEGABLAST=262,
    SB_BLOW=263,
    SB_FIREAURA=264,
    SB_RAPIDBLAST=265,
    SB_POISON=266,
    SB_SUPERBOOST=267,
    SB_CLAWSPEED=268,
    SB_CLAWAREA=269,
    SB_LOOT=270,
    SB_MENACE=271,
    SB_THREAT=272,
    SB_LOITER=273,
    SB_PUSH=274,
    SB_PURSETUG=275,
    SB_GROUNDPUNCH=276,
    SB_VANDALIZE=277,
    SB_RUNOUT=278,
    SB_SPRAYCAN=279,
    SB_THANK=280,
    SB_BOOSTAURA=281,
    SB_LOOKOUT=282,
    SB_CHEER=283,
    SB_DEALING=284,
    SB_RECEIVING=285,
    SB_OBSERVE=286,
    SB_TALK=287,
    SB_LISTEN=288,
    SB_ROLLDICE=289,
    SB_CHANTING=290,
    SB_BOWING=291,
    SB_MEDITATE=292,
    SB_MOMENTGLORY=293,
    SB_GUTPUNCH=294,
    SB_JUMPNSMASH=295,
    SB_BASEBALLPUNCH=296,
    SB_SHORTTOSS=297,
    SB_WINDUPPERCUT=298,
    SB_TAUNTA=299,
    SB_TAUNTB=200,
    SB_TAUNTC=301,
    SB_STOPCAST=302,
    SB_SCREAM=303,
    SB_TERRIFY=304,
    SB_ENERGYTORRENT=305,
    SB_HYPNOTIZE=306,
    SB_JUMPCLAW=307,
    SB_WINDUP=308,
    SB_MAGICWINDUP=309,
    SB_POWERWINDUP=310,
    SB_MYSICWINDUP=311,
    SB_TECHWINDUP=312,
    SB_POINT=313,
    SB_LAUGH=314,
    SB_SAD=315,
    SB_HAPPY=316,
    SB_ANGRY=317,
    SB_LOVE=318,
    SB_EVIL=319,
    SB_FLEX=320,
    SB_BECKON=321,
    SB_CLAP=322,
    SB_RUDE=323,
    SB_YES=324,
    SB_NO=325,
    SB_SHRUG=326,
    SB_DANCE=327,
    SB_SALUTE=328,
    SB_BUSY=329,
    SB_STOP=330,
    SB_WAIT=331,
    SB_HELP=332,
    SB_YELL=333,
    SB_CHAT=334,
    SB_COMMAND=335,
    SB_JOBSDONE=336,
    SB_PLAN=337,
    SB_WELDING=338,
    SB_WORK=339,
    SB_BODY=340,
    SB_BOULDER=341,
    SB_ZOMBIE=342,
    SB_KNEEL=343,
    SB_PLAYER=177,
    SB_RECLAIM=344,
    SB_CLIPBOARD=345,
    SB_BRIEFCASE=346,
    SB_CELLPHONE=347,
    SB_NEWSPAPER=348,
    SB_EAT=349,
    SB_CARPET=178,
    SB_INSEWERWATER=179,
    SB_SLIDING=180,
    SB_BOUNCING=181,
    SB_WARP=182,
    SB_RUNIN=183,
    SB_GAME=350,
    SB_RPS=351,
    SB_DICE=352,
    SB_ARGUE=353,
    SB_REASON=354,
    SB_QUESTION=355,
    SB_AFK=356,
    SB_ELEVATOR=357,
    SB_SCORE=358,
    SB_ONE=359,
    SB_TWO=360,
    SB_THREE=361,
    SB_FOUR=362,
    SB_FIVE=363,
    SB_SIX=364,
    SB_SEVEN=365,
    SB_EIGHT=366,
    SB_NINE=367,
    SB_TEN=368,
    SB_ZERO=369,
    SB_NON_EXISTING=~0U
};

struct SeqBitSet
{
    std::bitset<416> bits;
    bool isSet(SeqBitNames v) const { return bits[uint32_t(v)]; }
    void set(SeqBitNames bit) { bits[uint32_t(bit)] = true;}
    void setVal(SeqBitNames bit, bool v) { bits[uint32_t(bit)] = v; }
};

struct SeqMoveDataTypeAnim
{
    QString name;
    int firstFrame; // first frame ticks
    int lastFrame; // float ?
};

struct Parser_PlayFx
{
    QString name;
    uint32_t delay;
    uint32_t flags;
};

struct SeqMoveTypeData
{
    QString                            name;
    std::vector<SeqMoveDataTypeAnim *> m_Anim;
    std::vector<Parser_PlayFx *>       m_PlayFx;
    AnimTrack *                        anm_track;
    float                              Scale;
    float                              MoveRate;
    float                              PitchAngle;
    float                              PitchRate;
    float                              PitchStart;
    float                              PitchEnd;
    float                              SmoothSprint;
};

struct SeqNextMoveData
{
    QString name;
};

struct SeqCycleMoveData
{
    QString name;
};

enum
{
    MAXMOVES = 0x800
};

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
};

struct SeqMoveData
{
    QString name;
    float Scale;
    float MoveRate;
    uint32_t Interpolate;
    uint32_t Priority;
    uint32_t Flags;
    int idx;
    SeqMoveRawData raw;
    std::vector<SeqNextMoveData*> m_NextMove;
    std::vector<SeqCycleMoveData*> m_CycleMove;
    std::vector<SeqMoveTypeData*> m_Type;
    std::vector<QString > SticksOnChild;
    std::vector<QString > SetsOnChild;
    std::vector<QString > Sets;
    std::vector<QString > Requires;
    std::vector<QString > Member;
    std::vector<QString > Interrupts;

    enum eFlags
    {
        Cycle=1,
        GlobalAnim=2,
        FinishCycle=4,
        ReqInputs=8,
        ComplexCycle=0x10,
        NoInterp=0x20,
        HitReact=0x40,
        NoSizeScale=0x80,
        MoveScale=0x100,
        NotSelectable=0x200,
        SmoothSprint=0x400,
        PitchToTarget=0x800,
        PickRandomly=0x1000,
        FullSizeScale=0x2000,
        AlwaysSizeScale=0x4000
    };
};

struct SeqGroupNameData
{
    QString name;
};

struct SeqTypeDefData
{
    QString name;
    QString pBaseSkeleton;
    QString pParentType;
};

struct SequencerData
{
    QString name;
    std::vector<SeqTypeDefData *> m_TypeDef;
    std::vector<SeqGroupNameData *> m_Group;
    std::vector<SeqMoveData *> m_Move;
    int m_lastChangeDate;
    int initialized = 0;
};

struct SequencerList
{
    std::vector<SequencerData *> sq_list;
    int dev_seqInfoCount;
    QMap<QString, SequencerData> m_Sequencers; // ordered by sequencer's name ( implemented by 'operator <' )
};
