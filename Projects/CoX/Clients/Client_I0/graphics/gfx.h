#pragma once
#pragma pack(push, 1)
#include "renderer/RenderBonedModel.h"
#include <unordered_map>

struct GfxPrefs
{
  int screen_x;
  int screen_y;
  int screenX_pos;
  int screenY_pos;
  int maximized;
  int fullscreen;
  int mipLevel;
  float controls_draw_dist;
  float LODBias;
  float gamma;
  int disableSimpleShadows;
  int maxParticles;
  float maxParticleFill_div1mln;
  float fxSoundVolume;
  float musicSoundVolume;
  int enableVBOs;
};
#pragma pack(pop)
struct BeaconDbgRel
{
    Vector3 lineStart;
    Vector3 lineEnd;
    RGBA color;
    float linewidth;
};
enum GpuVendor
{
    ATI = 0x1002,
    NVIDIA = 0x10DE,
};
enum NvidiaCards
{
    GeForce3 = 0x200,
    Quadro_DCC = 0x203,
};
enum AtiCards
{
    Radeon_9000 = 0x4966,
    Radeon_9500_Pro = 0x4E45,
    Radeon_8500 = 0x514C,
    Radeon_9100 = 0x514D,
};

struct Parse_Sun
{
    char ****MoonName;
    Vector2 LampLightTime;
    Vector2 CloudFadeTime;
    float CloudFadeMin;
    Vector2 FogHeightRange;
    Vector2 FogDist;
};
struct Parse_SkyCloud
{
    char * Name;
    Vector2 Height;
    int ScrollRatio;
};
#pragma pack(push, 8)
struct Parse_SkyTime
{
    float time;
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 fogcolor;
    Vector3 highfogcolor;
    Vector3 backgroundcolor;
    Vector2 fogdist;
    float fogdepth;
    Vector4 shadowcolor;
    char *SkyName;
    GfxTree_Node *sky;
    Vector3 skypos;
    float moon_scales[8];
};
#pragma pack(pop)

struct Parse_Sky
{
    Parse_Sun **sky;
    int num_moons;
    Parse_SkyCloud **cloud;
    int num_clouds;
    Parse_SkyTime **skytime;
    int sun_time_count;
};
struct FogVals
{
    Vector2 startEnd;
    Vector3 color;
    char valid;
};
extern FogVals g_FogStartEnd;

struct SkyRel
{
    GfxTree_Node *node;
    GfxTree_Node *glow;
};
struct MemPool;
#pragma pack(push, 8)
struct Parsed_SkyScene
{
    char *CubeMap;
    char *Sky;
    int ClipFoggedGeometry;
    float FogRampColor;
    float FogRampDistance;
    int ScaleSpecularity;
    struct Parse_TexSwap **TexSwap;
};
#pragma pack(pop)
extern std::unordered_map<GfxTree_Node *, std::string> g_node_infos;
void segs_gfxUpdateFrame(bool force_render_world, bool head_shot);
void segs_sun_sunUpdate(int init);
extern void patch_gfx();
void segs_gfxSetViewMat(const Matrix4x3 *cam_mat, Matrix4x3 *view_mat, Matrix4x3 *inv_viewmat);
void segs_gfxDoNothingFrame();
void modelFreeAllCache(int arg0);
void segs_sunGlareDisable();
