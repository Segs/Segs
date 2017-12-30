#pragma once
#include "RenderBonedModel.h"
#include "utils/helpers.h"

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
struct ColorList
{
  RGBA field_0[16];
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
  int field_5C;
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
  int field_12C;
};
struct CameraInfo
{
  Matrix4x3 mat;
  Matrix4x3 cammat;
  Matrix4x3 viewmat;
  Matrix4x3 inv_viewmat;
  Vector3 lastPos;
  Vector3 angles;
  Vector3 targetPYR;
  Vector3 lastTargetPYR;
  Vector3 field_F0;
  int field_FC;
  int field_100;
  int field_104;
  int field_108;
  int field_10C;
  int field_110;
  float field_114;
  int field_118;
  float field_11C;
  int ( *camera_func)(CameraInfo *);
  char snap_pos;
  char field_125;
  __int16 field_126;
  float rotate_scale;
  float field_12C;
  float field_130;
  int field_134;
  int field_138;
};

#pragma pack(push, 1)
struct OptRel_A0
{
    int field_0;
    float map_time_of_day;
    float timescale;
    float timestepscale;
    int field_10[33];
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
extern void patch_rendertricks();
extern int segs_gfxNodeTricks(TrickNode *tricks, Model *model, Matrix4x3 *lhs);
extern void segs_modelSetAlpha(uint8_t alpha);
extern void segs_gfxNodeTricksUndo(TrickNode *trick, Model *model);