#include "gfx.h"

#include "utils/dll_patcher.h"
#include "renderer/RendererUtils.h"
#include "renderer/RendererState.h"
#include "renderer/RenderBonedModel.h"

#include "GL/glew.h"

#include <cstring>
#include <cassert>
#include <algorithm>
#include "renderer/Model.h"
#include "renderer/RenderTricks.h"
#include "GameState.h"

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
    Parse_Sun **sun;
    int num_suns;
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

extern "C" {
    __declspec(dllimport) void wcwMgmt_EnableFog(int);
    __declspec(dllimport) GfxTree_Node *gfxtree_Allocgfxtreenode(MemPool *pool);
    __declspec(dllimport) void fn_5B7BE0(MemPool *a1);
    __declspec(dllimport) MemPool *MemPool__Alloc();
    __declspec(dllimport) void MemPool__InitPool(MemPool *mp, size_t strct_size, int struct_count, const char *fl, int ln);
    __declspec(dllimport) void gfxtree_4ECDC0();
    __declspec(dllimport) GfxTree_Node *gfxtree_CreateEnqueNode(GfxTree_Node *parent_node, GfxTree_Node *curr, GfxTree_Node **head, GfxTree_Node **tail);
    __declspec(dllimport) GfxTree_Node *createGfxTreeNode_with_parent(GfxTree_Node *);
    __declspec(dllimport) char *fn_4BF9F0(const char *obj_name, char *path);
    __declspec(dllimport) int  VfPrintfWrapper(const char *fmt, ...);
    __declspec(dllimport) Model *anim_GeoReq4E9090(const char *model_name, const char *filename, char *shadowname, int load_type, int use_type);
    __declspec(dllimport) void gfxtree_4ECD00(GfxTree_Node *node);
    __declspec(dllimport) bool gfxTreeNodeIsValid(GfxTree_Node *node, int id);
    __declspec(dllimport) void gfxtree_gfxTreeDelete(GfxTree_Node *node);
    __declspec(dllimport) void fn_4FB720(const char *fmt, ...);
    __declspec(dllimport) char *strstri(const char *a1, const char *a2);
    __declspec(dllimport) void gfxNodeSetAlpha(GfxTree_Node *node, char alpha, int root_only);
    __declspec(dllimport) float normalizeRadAngle(float v);
    __declspec(dllimport) void  fn_5B6740(float yaw_angle, Matrix3x3 *tgt);
    __declspec(dllimport) void  fn_5B6840(float roll_angle, Matrix3x3 *tgt);
    __declspec(dllimport) void  pitchMat3(float pitch_angle, Matrix3x3 *tgt);
    __declspec(dllimport) void  Matrix3x3__scale(Matrix3x3 *src, Matrix3x3 *dst,float scale);

    __declspec(dllimport) int camera_is_inside;
    __declspec(dllimport) FogVals g_FogStartEnd;
    __declspec(dllimport) SunLight g_sun;
    __declspec(dllimport) OptRel_A0 server_visible_state;
    __declspec(dllimport) Parse_Sky parsed_sky;
    __declspec(dllimport) float g_TIMESTEP;
    __declspec(dllimport) GfxTree_Node * sky_gfx_tree_root;
    __declspec(dllimport) GfxTree_Node *gfxtree_node_7B8FDC; // sun node
    __declspec(dllimport) CameraInfo cam_info;
    __declspec(dllimport) FogVals struct_7B8DD8[2]; // indoor fog 
    __declspec(dllimport) FogVals struct_7B8E0C; // current fog
    __declspec(dllimport) Parsed_SkyScene parsed_scene;
}
static MemPool *s_skynode_pool;
static GfxTree_Node *sky_gfx_tree_tail;
static SkyRel celestial_objects[8];
static GfxTree_Node *s_clouds[8];
static int valid_tree_node_val;

// should be named: getDefaultPreferences()
void  segs_gfx_GfxCardFixes(GfxPrefs *settings)
{
    SysInfo_2 sys_info;
    int quality_mod = 3; // original was assuming 0
    //TODO: consider the fact that almost no one has 9500 or GeForce3 anymore
    //TODO: compare lowest end intel graphics card to Radeon 9500 and Geforce3, select default prefs for it
    segs_renderUtil_GetGfxCardVend(&sys_info);
    if ( sys_info.pci_ven == ATI )
    {
        if ( sys_info.pci_dev == Radeon_9500_Pro )
        {
            quality_mod = 3;
        }
        else if ( sys_info.pci_dev == Radeon_9100 ||
                  sys_info.pci_dev == Radeon_9000 ||
                  sys_info.pci_dev == Radeon_8500 )
        {
            quality_mod = 2;
        }
    }
    if ( sys_info.pci_ven == NVIDIA )
    {
        if ( sys_info.pci_dev < Quadro_DCC )
        {
            if ( sys_info.pci_dev < GeForce3 )
                quality_mod = 1;
            else
                quality_mod = 2;
        }
    }
    settings->fxSoundVolume = 1.0;
    settings->musicSoundVolume = 0.6f;
    settings->screenX_pos = 0;
    settings->screenY_pos = 0;
    settings->maximized = 0;
    settings->disableSimpleShadows = 1;
    settings->gamma = 1.0;
    settings->maxParticles = 50000;
    settings->maxParticleFill_div1mln = 10.0;
    settings->fullscreen = 1;
    settings->mipLevel = 0;

    switch(quality_mod)
    {
        case 0:
        case 2:
        settings->controls_draw_dist = 1.0;
        settings->LODBias = 1.0;
        settings->screen_x = 1024;
        settings->screen_y = 768;
        settings->enableVBOs = 1;
        break;
    case 1:
        settings->controls_draw_dist = 0.7f;
        settings->LODBias = 0.7f;
        settings->screen_x = 800;
        settings->screen_y = 600;
        settings->enableVBOs = 0;
        break;
    case 3:
        settings->controls_draw_dist = 1.0;
        settings->LODBias = 1.0;
        settings->screen_x = 1024;
        settings->screen_y = 768;
        settings->enableVBOs = 1;
        break;
    default:
        assert(!"Unknown quality value in segs_gfx_GfxCardFixes");
    }
    if ( sys_info.total_physical_memory <= 269000000 )
        settings->mipLevel = 1;
}

void  drawDebugBeacons(BeaconDbgRel *beacons, int count)
{
    segs_setupShading(DrawMode::COLORONLY, eBlendMode::MULTIPLY);
    glLineWidth(3.0);
    for(int idx=0; idx<count; ++idx)
    {
        const BeaconDbgRel &beacon(beacons[idx]);
        const float line_width = beacon.linewidth == 0.0f ? 1.0f : beacon.linewidth;
        glLineWidth(line_width);
        glColor4ub(beacon.color.r, beacon.color.b, beacon.color.g, beacon.color.a);
        glBegin(GL_LINES);
        glVertex3f(beacon.lineStart.x, beacon.lineStart.y, beacon.lineStart.z);
        glVertex3f(beacon.lineEnd.x, beacon.lineEnd.y, beacon.lineEnd.z);
        glEnd();
    }
    segs_modelDrawState(DrawMode::DUALTEX, 1);
    segs_modelBlendState(eBlendMode::MULTIPLY, 1);
}
static void drawColoredBox(float x1, float y1, float x2, float y2, uint32_t clr1, uint32_t clr2, uint32_t clr3, uint32_t clr4)
{
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);

    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    glBegin(GL_TRIANGLES);
        glColor4ub((clr3 & 0xFF0000) >> 16, (clr3 & 0xFF00) >> 8, clr3, (clr3 & 0xFF000000) >> 24);
        glVertex3f(x1, y1, -1.0f);
        glColor4ub((clr1 & 0xFF0000) >> 16, (clr1 & 0xFF00) >> 8, clr1, (clr1 & 0xFF000000) >> 24);
        glVertex3f(x1, y2, -1.0f);
        glColor4ub((clr2 & 0xFF0000) >> 16, (clr2 & 0xFF00) >> 8, clr2, (clr2 & 0xFF000000) >> 24);
        glVertex3f(x2, y2, -1.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glColor4ub((clr3 & 0xFF0000) >> 16, (clr3 & 0xFF00) >> 8, clr3, (clr3 & 0xFF000000) >> 24);
        glVertex3f(x1, y1, -1.0f);
        glColor4ub((clr2 & 0xFF0000u) >> 16, (clr2 & 0xFF00) >> 8, clr2, (clr2 & 0xFF000000) >> 24);
        glVertex3f(x2, y2, -1.0f);
        glColor4ub((clr4 & 0xFF0000u) >> 16, (clr4 & 0xFF00) >> 8, clr4, (clr4 & 0xFF000000) >> 24);
        glVertex3f(x2, y1, -1.0f);
    glEnd();
}
void drawFlatBox(int x1, int y1, int x2, int y2, uint32_t color)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 640.0, 0.0, 480.0, -1.0, 100.0);
    segs_texSetAllToWhite();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    wcwMgmt_EnableFog(0);
    glDisable(GL_DEPTH_TEST);
    segs_wcw_statemgmt_setDepthMask(true);
    glColor3f(1.0, 1.0, 1.0);
    segs_modelDrawState(DrawMode::SINGLETEX, 1);
    segs_modelBlendState(eBlendMode::MULTIPLY, 1);
    drawColoredBox(x1, y1, x2, y2, color, color, color, color);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void segs_setSunLight(Matrix4x3 *view_mat)
{
    Vector4 params;
    Matrix4x4 gl_mat;
    
    gl_mat = *view_mat;
    glLoadMatrixf(gl_mat.data());
    glEnable(GL_LIGHTING);
    segs_wcw_statemgmt_enableColorMaterial();
    glEnable(GL_LIGHT0);
    glColor4ub(0xFFu, 0xFFu, 0xFFu, 0xFFu);
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_AMBIENT, &g_sun.ambient);
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_DIFFUSE, &g_sun.diffuse);
    params.x = g_sun.direction.x;
    params.y = g_sun.direction.y;
    params.z = g_sun.direction.z;
    params.w = 0.0;
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_POSITION, &params);
    params = { 0,0,0,1 };
    segs_wcw_statemgmt_SetLightParam(GL_LIGHT0, GL_SPECULAR, &params);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &params.x);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &params.x);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
}
static float fixTime(float a1)
{
    while (a1 >= 24.0)
        a1 = a1 - 24.0;
    while (a1 < 0.0)
        a1 = a1 + 24.0;
    return a1;
}
static void gfxTreeInitSkyTree()
{
    if (s_skynode_pool)
        fn_5B7BE0(s_skynode_pool);
    s_skynode_pool = MemPool__Alloc();
    MemPool__InitPool(s_skynode_pool, sizeof(GfxTree_Node), 50, __FILE__, __LINE__);
    sky_gfx_tree_root = nullptr;
    sky_gfx_tree_tail = nullptr;
}
static GfxTree_Node * segs_gfxtree_CreateSkyGfxTreeRoot(GfxTree_Node *node)
{
    GfxTree_Node *curr = gfxtree_Allocgfxtreenode(s_skynode_pool);
    gfxtree_CreateEnqueNode(node, curr, &sky_gfx_tree_root, &sky_gfx_tree_tail);
    return curr;
}
GfxTree_Node *sunAddNode(const char *model_name, int which_graph_to_insert)
{
    GfxTree_Node *result;
    char filename[260] = {0};

    if (which_graph_to_insert)
        result = createGfxTreeNode_with_parent(nullptr);
    else
        result = segs_gfxtree_CreateSkyGfxTreeRoot(nullptr);
    fn_4BF9F0(model_name, filename);
    if (!filename[0])
        VfPrintfWrapper("\nSky builder can't find '%s' in the object_libray.\n", model_name);
    result->model = anim_GeoReq4E9090(model_name, filename, nullptr, 2, 0xA); //modelFind
    if (result->model)
        gfxtree_4ECD00(result);//gfxTreeInitGfxNodeWithObjectsTricks
    result->mat.r1 = { 1, 0, 0 };
    result->mat.r2 = { 0, 1, 0 };
    result->mat.r3 = { 0, 0, 1 };
    result->mat.TranslationPart = { 0, 0, 0 };
    return result;
}
void resetSunNode()
{
    if (gfxTreeNodeIsValid(gfxtree_node_7B8FDC, valid_tree_node_val))
        gfxtree_gfxTreeDelete(gfxtree_node_7B8FDC);
    gfxtree_node_7B8FDC = nullptr;
    valid_tree_node_val = 0;
}
static void initializeSky()
{
    char buf[260];

    gfxTreeInitSkyTree();
    g_FogStartEnd.valid = 0;
    memset(celestial_objects, 0, sizeof(celestial_objects));
    s_clouds[0] = nullptr;
    s_clouds[1] = nullptr;
    s_clouds[2] = nullptr;
    s_clouds[3] = nullptr;
    s_clouds[4] = nullptr;
    s_clouds[5] = nullptr;
    s_clouds[6] = nullptr;
    s_clouds[7] = nullptr;
    resetSunNode();
    for (int i = 0; i < parsed_sky.sun_time_count; ++i)
    {
        Parse_SkyTime *skytime = parsed_sky.skytime[i];
        if (skytime->SkyName)
        {
            skytime->sky = sunAddNode(skytime->SkyName, 0);
            if (parsed_sky.skytime[i]->sky)
            {
                skytime->skypos = skytime->sky->mat.TranslationPart;
            }
            else
            {
                fn_4FB720("Cant find sky: %s", skytime->SkyName);
            }
        }
        else
        {
            skytime->sky = nullptr;
        }
    }
    if (parsed_sky.num_suns)
    {
        char *name = ***(*parsed_sky.sun)->MoonName;
        celestial_objects[0].node = sunAddNode(name, 0);
        if (!celestial_objects[0].node)
            fn_4FB720("cant find sun: %s", name);
        strcpy(buf, name);
        char *underscores = strstr(buf, "__");
        if (underscores)
            *underscores = 0;
        strcat(buf,"glow__");
        celestial_objects[0].glow = sunAddNode(buf, 1);
        gfxtree_node_7B8FDC = celestial_objects[0].glow;
        valid_tree_node_val = celestial_objects[0].glow->unique_id;
    }
    for (int i = 1; i < parsed_sky.num_suns; ++i)
    {
        const char *moon_name = **(*parsed_sky.sun)->MoonName[i];
        celestial_objects[i].node = sunAddNode(moon_name, 0);
        if (!celestial_objects[i].node)
            fn_4FB720("cant find moon: %s", moon_name);
        celestial_objects[i].glow = nullptr;
    }
    for (int i = 0; i < parsed_sky.num_clouds; ++i)
    {
        const char *name = parsed_sky.cloud[i]->Name;
        s_clouds[i] = sunAddNode(name, 0);
        if (!s_clouds[i])
            fn_4FB720("cant find cloud: %s", name);
    }
}
static Vector3 LinearInterpolateVectors(Vector3 start, Vector3 end, float factor)
{
    return start * (1.0 - factor) + end * factor;
}
static float  LinearInterpolateValue(float start, float end, float factor)
{
    return start * (1.0 - factor) + end * factor;
}

static void fogBlend(FogVals *start, FogVals *end, float ratio, float feet, FogVals *result)
{
    result->color = LinearInterpolateVectors(start->color, end->color, ratio);
    for (int i = 0; i < 2; ++i)
    {
        float d = end->startEnd[i] - start->startEnd[i];
        float da = std::min(feet * g_TIMESTEP,std::abs(d));
        if (d < 0.0)
            da = -da;
        result->startEnd[i] = da + start->startEnd[i];
    }
}
static void indoorFogBlend()
{
    static float s_ratio;
    s_ratio = g_TIMESTEP * 0.03 + s_ratio;
    if (s_ratio > 6.283185307179586)
        s_ratio = s_ratio - 6.283185307179586;
    if (s_ratio >= 3.141592653589793)
        fogBlend(struct_7B8DD8, &struct_7B8DD8[1], 1.0, 3.0, &struct_7B8E0C);
    else
        fogBlend(struct_7B8DD8, &struct_7B8DD8[1], 0.0, 3.0, &struct_7B8E0C);
}
static void setSkyFog(Parse_SkyTime *early, Parse_SkyTime *late, float ratio)
{
    Vector2 fogdist; 
    int i;
    float fogheightratio;

    float fogdepth = LinearInterpolateValue(early->fogdepth,late->fogdepth,ratio);
    if ((*parsed_sky.sun)->FogHeightRange.x == 0.0)
        fogheightratio = 0.0;
    else
        fogheightratio = (cam_info.cammat.TranslationPart.y - (*parsed_sky.sun)->FogHeightRange.x) / ((*parsed_sky.sun)->FogHeightRange.y - (*parsed_sky.sun)->FogHeightRange.x);
    fogheightratio = std::max(0.0f,std::min(1.0f,fogheightratio));
    if (fogdepth < (*parsed_sky.sun)->FogDist.x)
        fogdepth = (1.0 - fogheightratio) * fogdepth + fogheightratio * (*parsed_sky.sun)->FogDist.x;
    if (early->fogdist.y != 0.0 || g_State.view.fogdist2 != 0.0)
    {
        for (i = 0; i < 2; ++i)
        {
            fogdist[i] = LinearInterpolateValue(early->fogdist[i], late->fogdist[i],ratio);
        }
        if (early->fogdist.y == 0.0)
        {
            fogdist.x = late->fogdist.x;
            fogdist.y = late->fogdist.y;
        }
        if (late->fogdist.y == 0.0)
        {
            fogdist.x = early->fogdist.x;
            fogdist.y = early->fogdist.y;
        }
        if (g_State.view.fogdist2 != 0.0)
        {
            fogdist.x = g_State.view.fogdist;
            fogdist.y = g_State.view.fogdist2;
        }
        for (i = 0; i < 2; ++i)
        {
            float t = int32_t((1.0 - fogheightratio) * fogdist[i] + fogheightratio * (*parsed_sky.sun)->FogDist[i]);
            fogdist[i] = std::min(t,fogdist[i]);
        }
        struct_7B8E0C.startEnd = fogdist;
    }
    Vector3 fogcolor = LinearInterpolateVectors(early->fogcolor, late->fogcolor,ratio);
    Vector3 highfogcolor = LinearInterpolateVectors(early->highfogcolor, late->highfogcolor, ratio); ;
    fogcolor = LinearInterpolateVectors(fogcolor, highfogcolor, fogheightratio); ;
    if (g_State.view.fogcolor.x != 0.0 || g_State.view.fogcolor.y != 0.0 || g_State.view.fogcolor.z != 0.0)
    {
        fogcolor = g_State.view.fogcolor * 0.0039215689f;
    }
    struct_7B8E0C.color = fogcolor;
}
void segs_sun_SetFog(Vector2 *param, const GLfloat *color)
{
    float drawdist = 1.0;
    if (g_State.view.controls_draw_dist < 1.0 && param->x > 400.0)
        drawdist = g_State.view.controls_draw_dist;
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, drawdist * param->x);
    glFogf(GL_FOG_END, drawdist * param->y);
    glFogfv(GL_FOG_COLOR, color);
}
static void fogBlendWithLast()
{
    if (!g_FogStartEnd.valid)
        fogBlend(&g_FogStartEnd, &struct_7B8E0C, 1.0, 50000.0, &g_FogStartEnd);
    fogBlend(&g_FogStartEnd, &struct_7B8E0C, parsed_scene.FogRampColor, parsed_scene.FogRampDistance, &g_FogStartEnd);
    g_FogStartEnd.valid = 1;
}
void __cdecl fixupCelestialObject(int idx, float add_dist, float rot, Vector3 *cam_pos, GfxTree_Node *node, float scale)
{
    struct CelestialFixups
    {
        float speed;
        Vector3 pyr;
    };
    const static CelestialFixups objects[9] = {
        { 1.0,{ 0.0,  1.9634954,  3.1415927 } },
        { 1.0,{ -0.61086524,  0.0,  0.0 } },
        { 1.0,{ 3.2288592,  1.3089969,  0.61086524 } },
        { 1.3,{ 3.1415927, -0.78539819,  0.0 } },
        { 2.0,{ 3.1415927,  1.5707964, -0.78539819 } },
        { 5.0,{ 0.08726646,  0.0,  1.3962634 } },
        { 0.5,{ 0.17453292,  1.2217305, -1.3962634 } },
        { 1.0,{ 0.2617994, -1.134464,  0.0 } },
        { 0.1,{ 0.0, -0.52359879,  0.6457718 } }
    };
    Matrix4x3 dest;
    Matrix4x3 to;
    if (node)
    {
        const CelestialFixups *dword_7B8E08 = &objects[idx];
        to.r1 = { 1,0,0 };
        to.r2 = { 0,1,0 };
        to.r3 = { 0,0,1 };
        to.TranslationPart = *cam_pos;
        fn_5B6740(dword_7B8E08->pyr.y, &to.ref3()); //yawMat3
        fn_5B6840(dword_7B8E08->pyr.z, &to.ref3()); //rollMat3
        float angle = rot * dword_7B8E08->speed - 3.141592653589793 + dword_7B8E08->pyr.x;
        pitchMat3(angle, &to.ref3());
        dest.r1 = { 1,0,0 };
        dest.r2 = { 0,1,0 };
        dest.r3 = { 0,0,1 };
        pitchMat3(-1.5707964, &dest.ref3());
        dest.TranslationPart.y = add_dist + 7500.0;
        dest.TranslationPart.x = 3000.0;
        node->mat = to * dest;
        Matrix3x3__scale(&node->mat.ref3(), &node->mat.ref3(), scale);
    }
}
static void setGlobalShadowColor(Vector4 *earlyShadowColor, Vector4 *lateShadowColor, float ratio)
{
    if (camera_is_inside)
    {
        g_sun.shadowcolor.x = 0.0;
        g_sun.shadowcolor.y = 0.0;
        g_sun.shadowcolor.z = 0.0;
        g_sun.shadowcolor.w = 0.35f;
    }
    else
    {
        g_sun.shadowcolor.x = (1.0 - ratio) * earlyShadowColor->x + ratio * lateShadowColor->x;
        g_sun.shadowcolor.y = (1.0 - ratio) * earlyShadowColor->y + ratio * lateShadowColor->y;
        g_sun.shadowcolor.z = (1.0 - ratio) * earlyShadowColor->z + ratio * lateShadowColor->z;
        g_sun.shadowcolor.w = (1.0 - ratio) * earlyShadowColor->w + ratio * lateShadowColor->w;
    }
}
void segs_sun_sunUpdate(int init)
{
    float v42; // [esp+4C8h] [ebp-68h]
    int time_idx; // [esp+4ECh] [ebp-44h]
    int nexttime_idx; // [esp+4F4h] [ebp-3Ch]
    float fogtimefade; // [esp+50Ch] [ebp-24h]
    Vector3 player_offset; // [esp+518h] [ebp-18h]
    float ratio; // [esp+52Ch] [ebp-4h]
    auto copyvs = server_visible_state;
    float _time = server_visible_state.timescale * g_TIMESTEP / 108000.0 + server_visible_state.map_time_of_day;
    _time = fixTime(_time);
    server_visible_state.map_time_of_day = _time;
    if (!parsed_sky.sun)
        return;

    assert(!sky_gfx_tree_root || sky_gfx_tree_root->unique_id > 0);
    player_offset.x = cam_info.cammat.TranslationPart.x;
    player_offset.y = 0.0;
    player_offset.z = cam_info.cammat.TranslationPart.z;
    if (init || !sky_gfx_tree_root)
        initializeSky();
    if (_time <= (*parsed_sky.sun)->CloudFadeTime.y)
        fogtimefade = std::min(1.0f,_time - ((*parsed_sky.sun)->CloudFadeTime.x - 1.0f));
    else
        fogtimefade = std::min(1.0f,(*parsed_sky.sun)->CloudFadeTime.y + 1.0f - _time);

    fogtimefade = std::max(fogtimefade, (*parsed_sky.sun)->CloudFadeMin / 256.0f);
    for (int i = 0; i < parsed_sky.num_clouds; ++i)
    {
        if (!s_clouds[i])
            continue;

        if (parsed_sky.cloud[i]->Height.y != 0.0)
        {
            float ratio = (cam_info.cammat.TranslationPart.y - parsed_sky.cloud[i]->Height.x) / (parsed_sky.cloud[i]->Height.y - parsed_sky.cloud[i]->Height.x);
            fogtimefade *= std::max(0.0f,std::min(1.0f,ratio));
        }
        if (s_clouds[i]->model)
        {
            if (!strstri(s_clouds[i]->model->bone_name_offset, "sunskirt"))
            {
                gfxNodeSetAlpha(s_clouds[i], fogtimefade * 255.0, 1);
            }
        }
        s_clouds[i]->mat.TranslationPart = player_offset;
    }
    assert(_time < 24.0 && _time >= 0.0);
    assert(parsed_sky.sun_time_count > 0 && parsed_sky.sun_time_count < 100);
    if (parsed_sky.sun_time_count == 1)
    {
        time_idx     = 0;
        nexttime_idx = 0;
    }
    else
    {
        bool success = false;
        for (int j = 0; j < parsed_sky.sun_time_count; ++j)
        {
            time_idx     = j;
            nexttime_idx = j + 1;
            if (nexttime_idx >= parsed_sky.sun_time_count)
                nexttime_idx    = 0;
            float time_val      = fixTime(parsed_sky.skytime[time_idx]->time);
            float next_time_val = fixTime(parsed_sky.skytime[nexttime_idx]->time);
            assert(time_val != next_time_val);
            if (time_val >= next_time_val)
            {
                if (_time >= time_val || _time < next_time_val)
                {
                    success = 1;
                }
            }
            else if (_time >= time_val && _time < next_time_val)
            {
                success = 1;
            }
            if (success)
                break;
        }
        assert(success);
    }
    Parse_SkyTime *early = parsed_sky.skytime[time_idx];
    Parse_SkyTime *late = parsed_sky.skytime[nexttime_idx];
    if (parsed_sky.sun_time_count <= 1)
    {
        ratio = 1.0;
    }
    else
    {
        float t_dist = fixTime(_time - early->time);
        float t_len  = fixTime(late->time - early->time);
        ratio        = t_len == 0.0 ? 0.0 : t_dist / t_len;
        ratio        = std::max(0.0f, std::min(1.0f, ratio));
    }
    for (int i = 0; i < parsed_sky.sun_time_count; ++i)
    {
        if (parsed_sky.skytime[i]->sky)
            gfxNodeSetAlpha(parsed_sky.skytime[i]->sky, 0, 1);
    }
    float r = ratio;
    if (early->sky && late->sky && early->sky->model == late->sky->model)
        r = 0.0;
    if (early->sky)
    {
        gfxNodeSetAlpha(early->sky, int32_t((1.0 - r) * 255.0), 1);
        early->sky->mat.TranslationPart = player_offset + early->skypos;
    }
    if (late->sky)
    {
        gfxNodeSetAlpha(late->sky, int32_t(r * 255.0), 1);
        late->sky->mat.TranslationPart = player_offset + late->skypos;
    }
    if (struct_7B8DD8[0].valid)
        indoorFogBlend();
    else
        setSkyFog(early, late, ratio);
    fogBlendWithLast();
    segs_sun_SetFog(&g_FogStartEnd.startEnd, &g_FogStartEnd.color.x);
    Vector3 bg_color = LinearInterpolateVectors(early->backgroundcolor, late->backgroundcolor, ratio);
    glClearColor(bg_color.x, bg_color.y, bg_color.z, 1.0);
    struct_7B8DD8[0].valid = 0;
    float rot              = (_time - 12.0) * 6.283185307179586 / 24.0;
    rot                    = normalizeRadAngle(rot);
    rot                    = rot * sqrt(fabs(rot)) / sqrt(3.141592653589793);
    for (int i = 0; i < parsed_sky.num_suns; ++i)
    {
        float scale = LinearInterpolateValue(early->moon_scales[i],late->moon_scales[i],ratio);
        if (celestial_objects[i].node)
            fixupCelestialObject(i, 0.0, rot, &player_offset, celestial_objects[i].node, scale);
        if (celestial_objects[i].glow)
            fixupCelestialObject(i, 0.0, rot, &player_offset, celestial_objects[i].glow, scale);
    }
    Parse_Sky copy = parsed_sky;
    if (_time <= (*parsed_sky.sun)->LampLightTime.y)
    {
        v42 = std::min(1.0f,_time - ((*parsed_sky.sun)->LampLightTime.x - 1.0f));
    }
    else
    {
        v42 = std::min((*parsed_sky.sun)->LampLightTime.y + 1.0f - _time , 1.0f);
    }
    g_sun.lamp_alpha = (1.0f - std::max(v42, 0.0f)) * 255.0f;
    setGlobalShadowColor(&early->shadowcolor, &late->shadowcolor, ratio);
    Vector3 ambient      = LinearInterpolateVectors(early->ambient, late->ambient, ratio);
    Vector3 diffuse      = LinearInterpolateVectors(early->diffuse, late->diffuse, ratio);
    g_sun.ambient.ref3() = ambient * 63.0;
    g_sun.diffuse.ref3() = diffuse * 63.0;
    g_sun.ambient.x      = g_sun.ambient.x * 0.0039215689;
    g_sun.ambient.y      = g_sun.ambient.y * 0.0039215689;
    g_sun.ambient.z      = g_sun.ambient.z * 0.0039215689;
    g_sun.diffuse.x      = g_sun.diffuse.x * 0.0039215689;
    g_sun.diffuse.y      = g_sun.diffuse.y * 0.0039215689;
    g_sun.diffuse.z      = g_sun.diffuse.z * 0.0039215689;
    if (celestial_objects[0].node)
    {
        g_sun.direction = celestial_objects[0].node->mat.TranslationPart - player_offset;
        g_sun.position  = celestial_objects[0].node->mat.TranslationPart;
    }
    g_sun.direction.ref3().normalize();
    if (g_sun.direction.y < 0.0)
        g_sun.direction.y        = -g_sun.direction.y;
    const float MAXIMUM_AMBIENT        = 1.0;
    const float MAXIMUM_PLAYER_AMBIENT = 1.0;
    const float MAXIMUM_DIFFUSE        = 1.0;
    const float MAXIMUM_PLAYER_DIFFUSE = 1.0;
    const float playerAmbientAdjuster  = 2.75;
    const float playerDiffuseAdjuster  = 0.52f;
    const float MINIMUM_AMBIENT        = 0.08f;
    const float MINIMUM_DIFFUSE        = 0.05f;
    const float MINIMUM_PLAYER_AMBIENT = 0.12f;
    const float MINIMUM_PLAYER_DIFFUSE = 0.06f;

    g_sun.no_angle_light.w      = 1.0;
    g_sun.ambient.w             = 1.0;
    g_sun.diffuse.w             = 1.0;
    g_sun.ambient_for_players.w = 1.0;
    g_sun.diffuse_for_players.w = 1.0;
    g_sun.ambient_for_players.x = g_sun.ambient.x * playerAmbientAdjuster;
    g_sun.ambient_for_players.y = g_sun.ambient.y * playerAmbientAdjuster;
    g_sun.ambient_for_players.z = g_sun.ambient.z * playerAmbientAdjuster;
    g_sun.diffuse_for_players.x = g_sun.diffuse.x * playerDiffuseAdjuster;
    g_sun.diffuse_for_players.y = g_sun.diffuse.y * playerDiffuseAdjuster;
    g_sun.diffuse_for_players.z = g_sun.diffuse.z * playerDiffuseAdjuster;
    for (int i = 0; i < 3; ++i)
    {
        g_sun.ambient[i] = std::max(MINIMUM_AMBIENT, g_sun.ambient[i]);
        g_sun.ambient_for_players[i] = std::max(MINIMUM_PLAYER_AMBIENT, g_sun.ambient_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.ambient[i] = std::min(MAXIMUM_AMBIENT, g_sun.ambient[i]);
        g_sun.ambient_for_players[i] = std::min(MAXIMUM_PLAYER_AMBIENT, g_sun.ambient_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.diffuse[i] = std::max(MINIMUM_DIFFUSE, g_sun.diffuse[i]);
        g_sun.diffuse_for_players[i] = std::max(MINIMUM_PLAYER_DIFFUSE, g_sun.diffuse_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.diffuse[i] = std::min(MAXIMUM_DIFFUSE, g_sun.diffuse[i]);
        g_sun.diffuse_for_players[i] = std::min(MAXIMUM_PLAYER_DIFFUSE, g_sun.diffuse_for_players[i]);
    }
    g_sun.no_angle_light.ref3() = g_sun.ambient.ref3() + g_sun.diffuse.ref3();
}

void segs_gfxTreeInit()
{
    gfxtree_4ECDC0();
    gfxTreeInitSkyTree();
}

void patch_gfx()
{
    PATCH_FUNC(gfx_GfxCardFixes);
    patchit("fn_4D8410", (void *)drawDebugBeacons);
    patchit("gfx_48AE20", (void *)drawFlatBox);
    patchit("gfxtree_4ECE30",(void *)segs_gfxTreeInit);
    patchit("fn_49C760", (void *)resetSunNode);
    patchit("gfxtree_4ECE30", (void *)segs_gfxTreeInit);
    PATCH_FUNC(setSunLight);
    PATCH_FUNC(sun_SetFog);
    PATCH_FUNC(sun_sunUpdate);

}
