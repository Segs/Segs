#pragma once

#include "GL/glew.h"
#include "utils/helpers.h"
#include "renderer/RenderBonedModel.h"

#pragma pack(push, 1)
struct SeqBitSet
{
    int masks[13];
};
#pragma pack(pop)
struct Parser_Sequencer;
struct Parser_Move;
struct Parser_MoveType;
struct AnimTrack;
struct ModelHeader;
struct Animation
{
    AnimTrack *baseAnimTrack;
    Parser_Move *lastMoveBeforeTriggeredMove;
};
struct SeqMove
{
    Parser_Move *move;
    Parser_Move *move_to_send;
    Parser_Move *prev_move;
    Parser_Move *move_lastframe;
    Parser_MoveType *type;
    float frame;
    float prev_frame;
};
struct SeqInstance_TrickStore
{
    int move_to_play;
    float triggertime;
    float currtime;
    int triggerFxNetId;
    char inuse;
};
enum ShadowType {
    SEQ_PLACE_SETBACK=1,
    SEQ_PLACE_DEAD_ON=2,
};
enum eShadowQuality
{
    SEQ_LOW_QUALITY_SHADOWS = 0x1,
    SEQ_MEDIUM_QUALITY_SHADOWS = 0x2,
    SEQ_HIGH_QUALITY_SHADOWS = 0x3,
};
enum eSeqTypeFlags
{
    SEQ_NO_SHALLOW_SPLASH = 0x1,
    SEQ_NO_DEEP_SPLASH = 0x2,
};
enum ePlacement {
    SEQ_SPLAT_SHADOW = 0x1,
    SEQ_NO_SHADOW = 0x2,
    SEQ_STENCIL_SHADOW = 0x3,
    SEQ_BIPED_SPLAT_SHADOW = 0x4,
};
enum eSelection {
    SEQ_SELECTION_BONES = 0x1,
    SEQ_SELECTION_COLLISION = 0x2,
    SEQ_SELECTION_WORLDGROUP = 0x3,
};
enum eCollision
{
    SEQ_COLLISION_0 = 0,
};
enum eConstantState
{
    SEQ_CONSTANT_STATE_0=0
};
static_assert(sizeof(SeqInstance_TrickStore) == 0x14);
#pragma pack(push, 8)
struct SeqType
{
    char name[128];
    char seqname[128];
    char seqTypeName[128];
    char gfxname[4][128];
    char Fx[5][128];
    char shadowTypeStr[50];
    enum ShadowType shadowType;
    char ShadowTexture[128];
    char shadowQualityStr[50];
    eShadowQuality shadowQuality;
    char flagsStr[50];
    eSeqTypeFlags flags;
    Vector3 ShadowSize;
    char Shadow[128];
    Vector3 ShadowOffset;
    int UseShadow;
    int TicksToLingerAfterDeath;
    int TicksToFadeAwayAfterDeath;
    int LightAsDoorOutside;
    float LOD0_Dist;
    float LOD1_Dist;
    float LOD2_Dist;
    float LOD3_Dist;
    float FadeOutStart;
    float FadeOutFinish;
    Vector3 GeomScale;
    Vector3 GeomScaleMax;
    float AnimScale;
    Vector3 CollisionSize;
    Vector3 CollisionOffset;
    float VisSphereRadius;
    int maxAlpha;
    float reverseFadeOutDist;
    char WorldGroup[128];
    float MinimumAmbient;
    char BoneScaleFat[128];
    char BoneScaleSkinny[128];
    int RandomBoneScale;
    int NotSelectable;
    int NoCollision;
    float Bounciness;
    char collisionStr[50];
    eCollision collision;
    char placementStr[50];
    ePlacement placement;
    float ReticleHeightBias;
    float ReticleWidthBias;
    char selectionStr[50];
    eSelection selection;
    char CapeFile[256];
    char CapeName[256];
    char CapeHarnessFile[256];
    char CapeHarnessName[256];
    char CapeTex1[256];
    char CapeTex2[256];
    char CapeInnerTex1[256];
    char CapeInnerTex2[256];
    Vector3 CapeColor1[4];
    char constantStr[50];
    eConstantState constant;
    int u_12D4;
    char unk12D8[8];
    int u_12E0;
    char unk12E4[32];
    int u_1304;
    int u_1308;
    int u_130C;
    int u_1310;
    int u_1314;
    int u_1318;
    SeqBitSet constantStateBits;
    int HasRandomName;
    int lastDate;
};
#pragma pack(pop)

struct SeqInstance
{
    GfxTree_Node *gfx_root;
    int gfx_root_unique_id;
    SeqBitSet state;
    SeqBitSet sticky_state;
    SeqBitSet state_lastframe;
    SeqType ent_type_info;
    Parser_Sequencer *server_orders;
    SeqMove m_anim;
    Animation animation;
    int triggeredMoveStartTime;
    int u_1428;
    Vector3 GeomScale;
    Vector3 fxSetGeomScale;
    float curranimscale;
    int move_predict_seed;
    char anim_incrementor;
    char u_144D;
    char u_144E;
    char u_144F;
    Vector3 posLastFrame;
    Handle seq_instance_handle;
    int loadingObjects;
    int updated_appearance;
    int u_1468;
    int lod_level;
    char instant_move;
    char u_1471;
    char u_1472;
    char u_1473;
    GlobRenderRel seqGfxData;
    int powerTargetEntId;
    Vector3 u_2568;
    Vector3 u_2574;
    float goalTargetingPitch;
    float currTargetingPitch;
    char bone_geometry_names[70][64];
    char bone_has_color[70];
    uint32_t bone_colors_1[70];
    uint32_t bone_colors_2[70];
    TextureBind *bone_textures1[70];
    TextureBind *bone_textures2[70];
    GLuint bone_rgb_buffers[70];
    int static_light_done;
    Handle othHandles[5];
    Handle const_seqfxHandles[6];
    Handle volume_fx;
    int volume_fx_type;
    int big_splash;
    Handle hndlWorldGroup;
    SeqInstance_TrickStore futuremovetrackers[4];
    int alphasvr;
    int alphacam;
    int alphadist;
    int alphafade;
    int alphafx;
    float BoneScaleRatio;
    ModelHeader *bonescale_anm_skinny;
    ModelHeader *bonescale_anm_fat;
    Vector3 bonescales[70];
    Vector4 last_rotations[70];
    Vector3 last_xlates[70];
    int last_fog_id[70];
    char curr_interp_state[70];
    int curr_interp_frame;
    int fog_id;
    GfxTree_Node *simpleShadow;
    int simpleShadowUniqueId;
    float maxDeathAlpha;
    Model *model;
    Model *model2;
    TextureBind *m_tex[4];
    int u_49F4[4];
    GfxTree_Node *clothnode;
    int clothnodeUniqueId;
    Vector3 u_4A0C;
    char unk_4A18[4];
};
static_assert(sizeof(SeqInstance) == 0x4A1C);
struct Entity
{
    Matrix4x3 mat4;
    char blob[0x164];
    SeqInstance *seq;
    char blob2[0x1F10 - 0x198];
    // this struct is larger than this, but for now only a few fields are needed in the patch code
};
static_assert(offsetof(Entity, seq) == 0x194);
static_assert(sizeof(Entity)==0x1F10);
void patch_entclient();
