#pragma once
#include "RenderBonedModel.h"
#include "utils/helpers.h"

struct MaterialDefinition;
enum TrickFlags : uint32_t
{
    TF_Additive = 0x1,
    TF_ScrollST0 = 0x2,
    TF_FaceFront = 0x4,
    TF_FaceCamera = 0x8,
    TF_DistAlpha = 0x10,
    TF_ColorOnly = 0x20,
    TF_DoubleSided = 0x40,
    TF_NoZTest = 0x80,
    ReflectTex1 = 0x100,
    TF_ScrollST1 = 0x200,
    TF_NightLight = 0x400,
    TF_NoZWrite = 0x800,
    TF_Wireframe = 0x1000,
    NoDraw = 0x2000,
    TF_STAnimate = 0x4000,
    TF_ParticleSys = 0x8000,
    NoColl = 0x10000,
    TF_SetColor = 0x20000,
    TF_VertexAlpha = 0x40000,
    NoFog = 0x80000,
    FogHasStartAndEnd = 0x100000,
    EditorVisible = 0x200000,
    CastShadow = 0x400000,
    TF_LightFace = 0x800000,
    ReflectTex0 = 0x1000000,
    TF_AlphaRef = 0x2000000,
    SimpleAlphaSort = 0x4000000,
    TF_TexBias = 0x8000000,
    TF_NightGlow = 0x10000000, // when set, light parameters are sent to gl
    SelectOnly = 0x20000000,
    TF_STSScale = 0x40000000,
    NotSelectable = 0x80000000,
};
struct TrickNode
{
    Vector2 ScrollST0;
    int u1;
    Vector2 ScrollST1;
    int u2;
    Vector3 tex_scale;
    int u4;
    int u5;
    RGBA TintColor0;
    RGBA TintColor1;
    uint32_t _TrickFlags;
    float SortBias;
    int u6;
    int u7;
    struct Parser_Trick *info;
};
static_assert(sizeof(TrickNode) == 0x48);
struct ColorList
{
    RGBA rgba_colors[16];
    int count;
    float scale;
};

struct Parser_Trick
{
    char *src_name;
    char *name;
    TrickNode node;
    int GfxFlags;
    int ObjFlags;
    int GroupFlags;
    int u_5C;
    float LodNear;
    float LodFar;
    float LodNearFade;
    float LodFarFade;
    struct Parser_StAnim **StAnim;
    Vector2 FogDist;
    float ShadowDist;
    float AlphaRef;
    float ObjTexBias;
    Vector2 NightGlow;
    float Sway;
    float Sway_Rotate;
    ColorList clists[2];
    float LodScale;
    int u_12C;
};
struct CameraInfo
{
    Matrix4x3 mat;
    Matrix4x3 cammat;
    Matrix4x3 viewmat;
    Matrix4x3 inv_viewmat;
    Vector3 lastPos;
    Vector3 angles;
    Vector3 prevPYR;
    Vector3 targetPYR;
    Vector3 prevTargetPYR;
    Vector3 velocityPYR;
    Vector3 prevVelocityPYR;
    Vector3 velocity;
    int ( *camera_func)(CameraInfo *);
    char snap_pos;
    char u_125;
    int16_t u_126;
    float rotate_scale;
    float lastZDist;
    float targetZDist;
    int last_height;
    int targetHeight;
};

#pragma pack(push, 1)
struct OptRel_A0
{
    int u_0;
    float map_time_of_day;
    float timescale;
    float timestepscale;
    int u_10[33];
    int pause;
    int disablegurneys;
    int timerel_9C;
};
#pragma pack(pop)
static_assert (sizeof(OptRel_A0) == 0xA0, "OptRel_A0");
struct SunTime
{
    float time;
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 fogcolor;
    Vector3 highfogcolor;
    Vector3 backgroundcolor;
    float fogdist[2];
    float fogdepth;
    float shadowcolor[4];
    char *skyname;
    GfxTree_Node *sky;
    Vector3 skypos;
    float moon_scales[8];
};
extern Vector4 g_tex_scrolls;

int segs_gfxNodeTricks(TrickNode *tricks, Model *model, Matrix4x3 *lhs,MaterialDefinition &def);
void segs_gfxNodeTricksUndo(TrickNode *trick, Model *model,const MaterialDefinition &def);
void segs_animateSts(Parser_StAnim *anim,struct InstanceDrawData &instance_data);
TrickNode *segs_getOrCreateTrickNode(Model *model);
extern void patch_rendertricks();
