#include "gfx.h"

#include "GameState.h"
#include "entity/entityDebug.h"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "graphics/GroupDraw.h"
#include "renderer/Model.h"
#include "renderer/RenderBonedModel.h"
#include "renderer/RenderModel.h"
#include "renderer/RenderParticles.h"
#include "renderer/RenderShadow.h"
#include "renderer/RenderSprites.h"
#include "renderer/RenderTree.h"
#include "renderer/RenderTricks.h"
#include "renderer/RendererState.h"
#include "renderer/RendererUtils.h"
#include "renderer/ShaderProgramCache.h"
#include "renderer/Texture.h"
#include "utils/dll_patcher.h"
#include "GL/glew.h"
#include "GL/wglew.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <windows.h>

struct Hash_Iter
{
    HashTable *tab;
    int        idx;
};
struct HashTab_Entry
{
    void *key;
    int   hash_val;
    void *value;
    int   u_C;
};
// std::unordered_map<GfxTree_Node *, std::string> g_node_infos; // used to record additional gfx tree node info, that
// cannot be put inside the object, only use this for debugging.
extern "C" {
__declspec(dllimport) void hashtab_first(HashTable *arg0, Hash_Iter *iter);
__declspec(dllimport) HashTab_Entry *hashtab_next(Hash_Iter *iter);
__declspec(dllimport) void *HashTable_GetEntryValue(HashTab_Entry *element);
__declspec(dllimport) void fn_5B7BE0(MemPool *pool);
__declspec(dllimport) MemPool *MemPool__Alloc();
__declspec(dllimport) void MemPool__InitPool(MemPool *mp, size_t strct_size, int struct_count, const char *fl, int ln);
__declspec(dllimport) void *getMemFromPool(MemPool *pool, const char *filename, int line);
__declspec(dllimport) GfxTree_Node *gfx_tree_RemoveNodeFromTree(GfxTree_Node *node, GfxTree_Node **head,
                                                                GfxTree_Node **tail);
__declspec(dllimport) GfxTree_Node *gfxtree_CreateEnqueNode(GfxTree_Node *parent_node, GfxTree_Node *curr,
                                                            GfxTree_Node **head, GfxTree_Node **tail);
__declspec(dllimport) GfxTree_Node *gfxtree_4ED460(GfxTree_Node *memtofree, MemPool *pool);
__declspec(dllimport) void gfxtree_FreeGfxNodeContents(GfxTree_Node *memtofree, MemPool *pool);
__declspec(dllimport) char *fn_4BF9F0(const char *obj_name, char *path);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt, ...);
__declspec(dllimport) Model *anim_GeoReq4E9090(const char *model_name, const char *filename, char *shadowname,
                                               int load_type, int use_type);
__declspec(dllimport) void gfxtree_4ECD00(GfxTree_Node *node);
__declspec(dllimport) bool gfxTreeNodeIsValid(GfxTree_Node *node, int id);
__declspec(dllimport) void fn_4FB720(const char *fmt, ...);
__declspec(dllimport) char *strstri(const char *str1, const char *str2);
__declspec(dllimport) float normalizeRadAngle(float v);
__declspec(dllimport) void fn_5B6740(float yaw_angle, Matrix3x3 *tgt);
__declspec(dllimport) void fn_5B6840(float roll_angle, Matrix3x3 *tgt);
__declspec(dllimport) void pitchMat3(float pitch_angle, Matrix3x3 *tgt);
__declspec(dllimport) void Matrix3x3__scale(Matrix3x3 *src, Matrix3x3 *dst, float scale);
__declspec(dllimport) void ReverseTransform(Matrix4x3 *, Matrix4x3 *);
__declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);
__declspec(dllimport) bool inEditMode();
__declspec(dllimport) void fx_fxRunEngine();
__declspec(dllimport) void gfx_PrintSeqInfo();
__declspec(dllimport) void drawStuffOnEntities();
__declspec(dllimport) void font_fontRender();
__declspec(dllimport) void DoorAnimCheckFade();
__declspec(dllimport) void renderUtil_rdrPixBufGetOld();
__declspec(dllimport) void gfx_48B1B0(const char *, int);
__declspec(dllimport) void fxCleanUp();
__declspec(dllimport) void showBgAdd(int diff);
__declspec(dllimport) void showBG(uint32_t);
__declspec(dllimport) void startFadeInScreen(int);
__declspec(dllimport) void statusLineDraw();
__declspec(dllimport) void gfx_showFramerate();
__declspec(dllimport) void printGfxSettings();
__declspec(dllimport) void displayEntDebugInfo();
__declspec(dllimport) void displaylog_4890F0();
__declspec(dllimport) void PrintSystemMessage(const char *fmt, ...);
__declspec(dllimport) int CohTimer__AllocTimer();
__declspec(dllimport) float CohTimer__elapsed(int idx);
__declspec(dllimport) int fn_4C13C0(const char *);
__declspec(dllimport) GroupDef *findNameInLib_P(const char *);
__declspec(dllimport) void CohTimer__ResetTimer(int);
__declspec(dllimport) void *fn_5B8FA0(HashTable *table, const char *key); // hashRemoveElement
__declspec(dllimport) void FreeModelGeometry(Model *model); // this should be named freeModelCollisionTriangles
__declspec(dllimport) void renderUtil_ReadPixel(float x, float y, GLvoid *pixels);
__declspec(dllimport) void get_screen_size(int *, int *);
__declspec(dllimport) void perspectiveTransform(Vector3 *threeD, Vector2 *twoD);

__declspec(dllimport) int int32_7B7E24;
__declspec(dllimport) int int32_6F13A8;
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) int glob_have_camera_pos;
__declspec(dllimport) int see_outside;
__declspec(dllimport) GfxTree_Node *gfx_tree_root;
__declspec(dllimport) OptRel_A0 server_visible_state;
__declspec(dllimport) Parse_Sky parsed_sky;
__declspec(dllimport) float g_TIMESTEP;
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) FogVals struct_7B8DD8[2]; // indoor fog
__declspec(dllimport) FogVals struct_7B8E0C;    // current fog
__declspec(dllimport) Parsed_SkyScene parsed_scene;
__declspec(dllimport) HDC device_context_handle;
__declspec(dllimport) int line_count;
__declspec(dllimport) BeaconDbgRel struct_114C3C0[50000];
__declspec(dllimport) BeaconDbgRel *int32_114C3AC;
__declspec(dllimport) int beacon_dbg_idx;
__declspec(dllimport) int int32_D49520;
__declspec(dllimport) BeaconDbgRel struct_D41820[1000];
__declspec(dllimport) MemPool *MP_GfxNode;
__declspec(dllimport) GfxTree_Node *gfx_tree_root_tail;
__declspec(dllimport) HashTable *GeoHashTable;
__declspec(dllimport) int maxSimultaneousFx;
__declspec(dllimport) int fxDestroyedCount;
__declspec(dllimport) int fxCreatedCount;
__declspec(dllimport) int seqLoadInstCalls;
__declspec(dllimport) int seqFreeInstCalls;
}
static MemPool *                     s_skynode_pool;
static GfxTree_Node *                sky_gfx_tree_root;
static GfxTree_Node *                sky_gfx_tree_tail;
static SkyRel                        celestial_objects[8];
static std::array<GfxTree_Node *, 8> s_clouds;
static int                           valid_tree_node_val;
static GeometryData                  simple_quad_vbo;
static int                           startTypeDrawing;
static int                           startAlphaDrawing;
static int                           startShadowDrawing;
static int                           endDrawModels;
static int                           s_blend_mode_switches;
static GfxTree_Node *                g_sun_gfx_node; // sun node

SunLight g_sun;
FogVals  g_FogStartEnd;

void          lightLines2(Matrix4x3 *mat, MaterialDefinition &material);
void          drawLightDirLines(Matrix4x3 *mat, MaterialDefinition &material);
GfxTree_Node *segs_createGfxTreeNode_with_parent(GfxTree_Node *);
GfxTree_Node *segs_gfxtree_Allocgfxtreenode(MemPool *pool);
void          segs_gfxtree_gfxTreeDelete(GfxTree_Node *node);
static void   resetViewspaceMatCount()
{
    viewspaceMatCount = 0;
}
void segs_gfxNodeSetAlpha(GfxTree_Node *node, int alpha, int skip_sibling_nodes)
{
    for (GfxTree_Node *i = node; i; i = i->next)
    {
        i->setAlpha(alpha);
        if (i->children_list)
            segs_gfxNodeSetAlpha(i->children_list, alpha, 0);
        if (skip_sibling_nodes)
            break;
    }
}

// should be named: getDefaultPreferences()
void segs_gfx_GfxCardFixes(GfxPrefs *settings)
{
    SysInfo_2 sys_info;
    // TODO: consider the fact that almost no one has 9500 or GeForce3 anymore
    // TODO: compare lowest end intel graphics card to Radeon 9500 and Geforce3, select default prefs for it
    segs_renderUtil_GetGfxCardVend(&sys_info);
    settings->fxSoundVolume           = 1.0;
    settings->musicSoundVolume        = 0.6f;
    settings->screenX_pos             = 0;
    settings->screenY_pos             = 0;
    settings->maximized               = 0;
    settings->disableSimpleShadows    = 1;
    settings->gamma                   = 1.0;
    settings->maxParticles            = 50000;
    settings->maxParticleFill_div1mln = 10.0;
    settings->fullscreen              = 1;
    settings->mipLevel                = 0;
    settings->controls_draw_dist      = 1.0;
    settings->LODBias                 = 1.0;
    settings->screen_x                = 1024;
    settings->screen_y                = 768;
    settings->enableVBOs              = 1;
    if (sys_info.total_physical_memory <= 269000000)
        settings->mipLevel = 1;
}
static void drawDebugBeacons(BeaconDbgRel *beacons, int count, MaterialDefinition &material)
{
    glLineWidth(3.0);
    //    std::vector<uint8_t> colors;
    //    std::vector<Vector3> pos;
    //    std::vector<uint16_t> indices;
    //    uint16_t index_offset=0;
    // TODO: check if all beacon.line widths are same, or sort beacons by line width, and do separate draw calls
    for (int idx = 0; idx < count; ++idx)
    {

        const BeaconDbgRel &beacon(beacons[idx]);
        const float         line_width = beacon.linewidth == 0.0f ? 1.0f : beacon.linewidth;
        glLineWidth(line_width);
        glColor4ub(beacon.color.r, beacon.color.b, beacon.color.g, beacon.color.a);
        glBegin(GL_LINES);
        glVertex3f(beacon.lineStart.x, beacon.lineStart.y, beacon.lineStart.z);
        glVertex3f(beacon.lineEnd.x, beacon.lineEnd.y, beacon.lineEnd.z);
        glEnd();
    }
    material.apply();
}
void drawDebugBeaconsWithMatrix(BeaconDbgRel *beacons, int count, Matrix4x3 *mat,
                                MaterialDefinition &material) // gfx_4D8580
{
    material.draw_data.modelViewMatrix = mat->toGLM();
    material.apply();
    drawDebugBeacons(beacons, count, material);
}
static void drawColoredBox(float x1, float y1, float x2, float y2, uint32_t clr1, uint32_t clr2, uint32_t clr3,
                           uint32_t clr4, MaterialDefinition &material)
{
    if (!simple_quad_vbo.segs_data)
        simple_quad_vbo.createVAO();
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
    float vbo_data[] = {
        x1, y1, -1.0f, x1, y2, -1.0f, x2, y2, -1.0f, x2, y1, -1.0f,
    };
    uint8_t colors[] = {
        uint8_t(clr3 >> 16), uint8_t(clr3 >> 8), uint8_t(clr3), uint8_t(clr3 >> 24),
        uint8_t(clr1 >> 16), uint8_t(clr1 >> 8), uint8_t(clr1), uint8_t(clr1 >> 24),
        uint8_t(clr2 >> 16), uint8_t(clr2 >> 8), uint8_t(clr2), uint8_t(clr2 >> 24),
        uint8_t(clr4 >> 16), uint8_t(clr4 >> 8), uint8_t(clr4), uint8_t(clr4 >> 24),
    };
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};
    simple_quad_vbo.uploadVerticesToBuffer(vbo_data, 12);
    simple_quad_vbo.uploadColorsToBuffer(colors, 16);
    simple_quad_vbo.uploadIndicesToBuffer(indices, 6);
    material.apply();
    simple_quad_vbo.draw(*material.program, GL_TRIANGLES, 6, 0);
}
void drawFlatBox(int x1, int y1, int x2, int y2, uint32_t color)
{
    GLDebugGuard       guard(__FUNCTION__);
    auto               restore = g_render_state.getGlobal();
    MaterialDefinition flat_material(g_default_mat);
    flat_material.setDrawMode(DrawMode::COLORONLY);
    flat_material.setFragmentMode(eBlendMode::MULTIPLY);

    segs_texSetAllToWhite();
    flat_material.draw_data.projectionMatrix   = glm::ortho(0.0, 640.0, 0.0, 480.0, -1.0, 100.0);
    flat_material.draw_data.modelViewMatrix    = glm::mat4(1);
    flat_material.draw_data.light0.State       = false;
    flat_material.draw_data.globalColor        = {1, 1, 1, 1};
    flat_material.draw_data.fog_params.enabled = false;
    flat_material.render_state.setDepthTestMode(RenderState::CMP_NONE);
    flat_material.render_state.setDepthWrite(true);
    flat_material.apply();
    drawColoredBox(x1, y1, x2, y2, color, color, color, color, flat_material);
    g_render_state.apply(restore);
}
void segs_ttDrawBoxBasic(float x1, float y1, float x2, float y2, uint32_t clr2, uint32_t clr3, uint32_t clr1,
                         uint32_t clr4, int draw_quad)
{
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, 0);
    GeometryData fakevbo;
    float        vbo_data[] = {
        x1, y1, -1, x1, y2, -1, x2, y2, -1, x2, y1, -1,
    };
    uint8_t  colors[]       = {uint8_t(clr1 >> 16), uint8_t(clr1 >> 8), uint8_t(clr1), uint8_t(clr1 >> 24),
                        uint8_t(clr2 >> 16), uint8_t(clr2 >> 8), uint8_t(clr2), uint8_t(clr2 >> 24),
                        uint8_t(clr3 >> 16), uint8_t(clr3 >> 8), uint8_t(clr3), uint8_t(clr3 >> 24),
                        uint8_t(clr4 >> 16), uint8_t(clr4 >> 8), uint8_t(clr4), uint8_t(clr4 >> 24)};
    uint32_t quad_indices[] = {0, 1, 2, 0, 2, 3};
    uint32_t line_indices[] = {0, 1, 2, 3};

    simple_quad_vbo.uploadVerticesToBuffer(vbo_data, 12);
    simple_quad_vbo.uploadColorsToBuffer(colors, 16);
    simple_quad_vbo.uploadIndicesToBuffer(draw_quad ? quad_indices : line_indices, draw_quad ? 6 : 4);
    MaterialDefinition temp_material(g_default_mat);
    temp_material.setDrawMode(DrawMode::SINGLETEX);
    temp_material.setFragmentMode(eBlendMode::MULTIPLY);

    temp_material.apply();
    GLenum mode = draw_quad ? GL_TRIANGLES : GL_LINE_LOOP;
    simple_quad_vbo.draw(*temp_material.program, mode, draw_quad ? 6 : 4, 0);
}
void segs_setSunLight(Matrix4x3 *view_mat)
{
    g_light_state.State    = true;
    g_light_state.Ambient  = g_sun.ambient;
    g_light_state.Diffuse  = g_sun.diffuse;
    g_light_state.Position = view_mat->ref3() * g_sun.direction.ref3();
    g_light_state.Specular = {0, 0, 0, 1};
    // TODO: set gloss exponent to 128 here
}
void segs_sunGlareDisable()
{
    if (g_sun_gfx_node)
        segs_gfxNodeSetAlpha(g_sun_gfx_node, 0, 1);
}
void segs_sunVisible()
{
    Vector3 dst;
    int     width;
    int     height;
    Vector2 screen_pos;
    uint8_t marking_value;

    int change_direction = 0;
    if (!g_sun_gfx_node)
        return;

    Vector3 src = g_sun_gfx_node->mat.TranslationPart;
    if (src.y >= cam_info.cammat.TranslationPart.y)
    {
        dst = cam_info.viewmat * src;
        perspectiveTransform(&dst, &screen_pos);
        get_screen_size(&width, &height);
        if (screen_pos.x >= 0.0f && width > screen_pos.x && screen_pos.y >= 0.0f && height > screen_pos.y)
        {
            float depth;
            renderUtil_ReadPixel(screen_pos.x, screen_pos.y, &depth);
            if (depth > 0.99989998f)
                change_direction = 1;
        }
    }
    marking_value = g_sun_gfx_node->alpha();
    if (change_direction)
        marking_value = std::min(255, std::max<int>(0, 40 * g_TIMESTEP + marking_value));
    else
        marking_value = std::min(255, std::max<int>(0, marking_value - 40 * g_TIMESTEP));
    segs_gfxNodeSetAlpha(g_sun_gfx_node, marking_value, 1);
}
static float fixTime(float tm_val)
{
    while (tm_val >= 24.0f)
        tm_val = tm_val - 24.0f;
    while (tm_val < 0.0f)
        tm_val = tm_val + 24.0f;
    return tm_val;
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
static GfxTree_Node *segs_gfxtree_CreateSkyGfxTreeRoot(GfxTree_Node *node)
{
    GfxTree_Node *curr = segs_gfxtree_Allocgfxtreenode(s_skynode_pool);
    gfxtree_CreateEnqueNode(node, curr, &sky_gfx_tree_root, &sky_gfx_tree_tail);
    return curr;
}
GfxTree_Node *sunAddNode(const char *model_name, int which_graph_to_insert)
{
    GfxTree_Node *result;
    char          filename[260] = {0};

    if (which_graph_to_insert)
        result = segs_createGfxTreeNode_with_parent(nullptr);
    else
        result = segs_gfxtree_CreateSkyGfxTreeRoot(nullptr);
    // g_node_infos[result] = model_name;
    fn_4BF9F0(model_name, filename);
    if (!filename[0])
        VfPrintfWrapper("\nSky builder can't find '%s' in the object_libray.\n", model_name);
    result->model = anim_GeoReq4E9090(model_name, filename, nullptr, 2, 10); // modelFind
    if (result->model)
        gfxtree_4ECD00(result); // gfxTreeInitGfxNodeWithObjectsTricks
    result->mat = Unity_Matrix;
    return result;
}
void resetSunNode()
{
    if (gfxTreeNodeIsValid(g_sun_gfx_node, valid_tree_node_val))
        segs_gfxtree_gfxTreeDelete(g_sun_gfx_node);
    g_sun_gfx_node      = nullptr;
    valid_tree_node_val = 0;
}
static void initializeSky()
{
    char buf[260];

    gfxTreeInitSkyTree();
    g_FogStartEnd.valid = 0;
    memset(celestial_objects, 0, sizeof(celestial_objects));
    s_clouds.fill(nullptr);
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
            } else
            {
                fn_4FB720("Cant find sky: %s", skytime->SkyName);
            }
        }
        else
        {
            skytime->sky = nullptr;
        }
    }
    if (parsed_sky.num_moons)
    {
        char *name  = ***parsed_sky.sky[0]->MoonName;
        celestial_objects[0].node = sunAddNode(name, 0);
        if (!celestial_objects[0].node)
            fn_4FB720("cant find sun: %s", name);
        strcpy(buf, name);
        char *underscores = strstr(buf, "__");
        if (underscores)
            *underscores = 0;
        strcat(buf, "glow__");
        celestial_objects[0].glow = sunAddNode(buf, 1);
        g_sun_gfx_node            = celestial_objects[0].glow;
        valid_tree_node_val       = celestial_objects[0].glow->unique_id;
    }
    for (int i = 1; i < parsed_sky.num_moons; ++i)
    {
        const char *moon_name     = **(parsed_sky.sky[0])->MoonName[i];
        celestial_objects[i].node = sunAddNode(moon_name, 0);
        if (!celestial_objects[i].node)
            fn_4FB720("cant find moon: %s", moon_name);
        celestial_objects[i].glow = nullptr;
    }
    for (int i = 0; i < parsed_sky.num_clouds; ++i)
    {
        const char *name = parsed_sky.cloud[i]->Name;
        s_clouds[i]      = sunAddNode(name, 0);
        if (!s_clouds[i])
            fn_4FB720("cant find cloud: %s", name);
    }
}
static Vector3 LinearInterpolateVectors(Vector3 start, Vector3 end, float factor)
{
    return start * (1.0f - factor) + end * factor;
}
static float LinearInterpolateValue(float start, float end, float factor)
{
    return start * (1.0f - factor) + end * factor;
}

static void fogBlend(FogVals *start, FogVals *end, float ratio, float feet, FogVals *result)
{
    result->color = LinearInterpolateVectors(start->color, end->color, ratio);
    for (int i = 0; i < 2; ++i)
    {
        float d  = end->startEnd[i] - start->startEnd[i];
        float da = std::min(feet * g_TIMESTEP, std::abs(d));
        if (d < 0.0f)
            da = -da;
        result->startEnd[i] = da + start->startEnd[i];
    }
}
static void indoorFogBlend()
{
    static float s_ratio;
    s_ratio = g_TIMESTEP * 0.03f + s_ratio;
    if (s_ratio > float(2 * M_PI))
        s_ratio = s_ratio - float(2 * M_PI);
    if (s_ratio >= float(M_PI))
        fogBlend(struct_7B8DD8, &struct_7B8DD8[1], 1.0, 3.0, &struct_7B8E0C);
    else
        fogBlend(struct_7B8DD8, &struct_7B8DD8[1], 0.0, 3.0, &struct_7B8E0C);
}
static void setSkyFog(Parse_SkyTime *early, Parse_SkyTime *late, float ratio)
{
    Vector2 fogdist;
    int     i;
    float   fogheightratio;

    float fogdepth = LinearInterpolateValue(early->fogdepth, late->fogdepth, ratio);
    if ((*parsed_sky.sky)->FogHeightRange.x == 0.0f)
        fogheightratio = 0.0;
    else
        fogheightratio = (cam_info.cammat.TranslationPart.y - (*parsed_sky.sky)->FogHeightRange.x) /
                         ((*parsed_sky.sky)->FogHeightRange.y - (*parsed_sky.sky)->FogHeightRange.x);
    fogheightratio = std::max(0.0f, std::min(1.0f, fogheightratio));
    if (fogdepth < (*parsed_sky.sky)->FogDist.x)
        fogdepth = (1.0f - fogheightratio) * fogdepth + fogheightratio * (*parsed_sky.sky)->FogDist.x;
    if (early->fogdist.y != 0.0f || g_State.view.fogdists.y != 0.0f)
    {
        for (i = 0; i < 2; ++i)
        {
            fogdist[i] = LinearInterpolateValue(early->fogdist[i], late->fogdist[i], ratio);
        }
        if (early->fogdist.y == 0.0f)
        {
            fogdist = late->fogdist;
        }
        if (late->fogdist.y == 0.0f)
        {
            fogdist = early->fogdist;
        }
        if (g_State.view.fogdists.y != 0.0f)
        {
            fogdist = g_State.view.fogdists;
        }
        for (i = 0; i < 2; ++i)
        {
            float t    = int32_t((1.0f - fogheightratio) * fogdist[i] + fogheightratio * (*parsed_sky.sky)->FogDist[i]);
            fogdist[i] = std::min(t, fogdist[i]);
        }
        struct_7B8E0C.startEnd = fogdist;
    }
    Vector3 fogcolor     = LinearInterpolateVectors(early->fogcolor, late->fogcolor, ratio);
    Vector3 highfogcolor = LinearInterpolateVectors(early->highfogcolor, late->highfogcolor, ratio);
    ;
    fogcolor = LinearInterpolateVectors(fogcolor, highfogcolor, fogheightratio);
    ;
    if (g_State.view.fogcolor.x != 0.0f || g_State.view.fogcolor.y != 0.0f || g_State.view.fogcolor.z != 0.0f)
    {
        fogcolor = g_State.view.fogcolor * (1.0f / 255.0f);
    }
    struct_7B8E0C.color = fogcolor;
}
void segs_sun_SetFog(Vector2 *param, const GLfloat *color)
{
    float drawdist = 1.0;
    if (g_State.view.vis_scale < 1.0f && param->x > 400.0f)
        drawdist = g_State.view.vis_scale;
    g_fog_state.setMode(1);
    g_fog_state.color = {color[0], color[1], color[2], color[3]};
    g_fog_state.setStart(drawdist * param->x);
    g_fog_state.setEnd(drawdist * param->y);
}
static void fogBlendWithLast()
{
    if (!g_FogStartEnd.valid)
        fogBlend(&g_FogStartEnd, &struct_7B8E0C, 1.0, 50000.0, &g_FogStartEnd);
    fogBlend(&g_FogStartEnd, &struct_7B8E0C, parsed_scene.FogRampColor, parsed_scene.FogRampDistance, &g_FogStartEnd);
    g_FogStartEnd.valid = 1;
}
static constexpr float degToRad(float v)
{
    return glm::pi<float>() * v / 180.0f;
}
void fixupCelestialObject(int idx, float add_dist, float rot, Vector3 *cam_pos, GfxTree_Node *node, float scale)
{
    struct CelestialFixups
    {
        float   speed;
        Vector3 pyr;
    };
    const static CelestialFixups objects[9] = {
        {1.0f, {degToRad(0), degToRad(112.5f), degToRad(180.0f)}}, {1.0f, {-degToRad(35.0), degToRad(0), 0.0}},
        {1.0f, {degToRad(185), degToRad(75), degToRad(35.0)}},     {1.3f, {degToRad(180), -degToRad(45), 0.0}},
        {2.0f, {degToRad(180), degToRad(90), -degToRad(45)}},      {5.0f, {degToRad(5), degToRad(0), degToRad(80)}},
        {0.5f, {degToRad(10), degToRad(70), -degToRad(80)}},       {1.0f, {degToRad(15), -degToRad(65), 0.0}},
        {0.1f, {degToRad(0), -degToRad(30), degToRad(37)}}};
    Matrix4x3 dest;
    Matrix4x3 to;
    if (node)
    {
        const CelestialFixups *selected_body_data = &objects[idx];
        to                                        = Unity_Matrix;
        to.TranslationPart                        = *cam_pos;
        fn_5B6740(selected_body_data->pyr.y, &to.ref3()); // yawMat3
        fn_5B6840(selected_body_data->pyr.z, &to.ref3()); // rollMat3
        float angle = rot * selected_body_data->speed - glm::pi<float>() + selected_body_data->pyr.x;
        pitchMat3(angle, &to.ref3());
        dest = Unity_Matrix;
        pitchMat3(-degToRad(90), &dest.ref3());
        dest.TranslationPart.y = add_dist + 7500.0f;
        dest.TranslationPart.x = 3000.0;
        node->mat              = to * dest;
        Matrix3x3__scale(&node->mat.ref3(), &node->mat.ref3(), scale);
    }
}
static void setGlobalShadowColor(Vector4 *earlyShadowColor, Vector4 *lateShadowColor, float ratio)
{
    if (camera_is_inside)
    {
        g_sun.shadowcolor = {0.0,0.0,0.0,0.35f};
    }
    else
    {
        g_sun.shadowcolor.x = (1.0f - ratio) * earlyShadowColor->x + ratio * lateShadowColor->x;
        g_sun.shadowcolor.y = (1.0f - ratio) * earlyShadowColor->y + ratio * lateShadowColor->y;
        g_sun.shadowcolor.z = (1.0f - ratio) * earlyShadowColor->z + ratio * lateShadowColor->z;
        g_sun.shadowcolor.w = (1.0f - ratio) * earlyShadowColor->w + ratio * lateShadowColor->w;
    }
}
void segs_sun_sunUpdate(int init)
{
    float   tmp;
    int     time_idx     = 0;
    int     nexttime_idx = 0;
    float   fogtimefade;
    Vector3 player_offset;
    float   ratio;

    float _time = server_visible_state.timescale * g_TIMESTEP / 108000.0f + server_visible_state.map_time_of_day;
    _time       = fixTime(_time);
    server_visible_state.map_time_of_day = _time;
    if (!parsed_sky.sky)
        return;

    assert(!sky_gfx_tree_root || sky_gfx_tree_root->unique_id > 0);
    player_offset.x = cam_info.cammat.TranslationPart.x;
    player_offset.y = 0.0;
    player_offset.z = cam_info.cammat.TranslationPart.z;
    if (init || !sky_gfx_tree_root)
        initializeSky();
    if (_time <= (*parsed_sky.sky)->CloudFadeTime.y)
        fogtimefade = std::min(1.0f, _time - ((*parsed_sky.sky)->CloudFadeTime.x - 1.0f));
    else
        fogtimefade = std::min(1.0f, (*parsed_sky.sky)->CloudFadeTime.y + 1.0f - _time);

    fogtimefade = std::max(fogtimefade, (*parsed_sky.sky)->CloudFadeMin / 256.0f);
    for (int i = 0; i < parsed_sky.num_clouds; ++i)
    {
        if (!s_clouds[i])
            continue;

        if (parsed_sky.cloud[i]->Height.y != 0.0f)
        {
            float fog_ratio = (cam_info.cammat.TranslationPart.y - parsed_sky.cloud[i]->Height.x) /
                              (parsed_sky.cloud[i]->Height.y - parsed_sky.cloud[i]->Height.x);
            fogtimefade *= std::max(0.0f, std::min(1.0f, fog_ratio));
        }
        if (s_clouds[i]->model)
        {
            if (!strstri(s_clouds[i]->model->bone_name_offset, "sunskirt"))
            {
                segs_gfxNodeSetAlpha(s_clouds[i], uint8_t(fogtimefade * 255), 1);
            }
        }
        s_clouds[i]->mat.TranslationPart = player_offset;
    }
    assert(_time < 24.0f && _time >= 0.0f);
    assert(parsed_sky.sun_time_count > 0 && parsed_sky.sun_time_count < 100);
    if (parsed_sky.sun_time_count == 1)
    {
        time_idx     = 0;
        nexttime_idx = 0;
    } else
    {
        bool success = false;
        for (int j = 0; j < parsed_sky.sun_time_count; ++j)
        {
            time_idx     = j;
            nexttime_idx = j + 1;
            if (nexttime_idx >= parsed_sky.sun_time_count)
                nexttime_idx = 0;
            float time_val      = fixTime(parsed_sky.skytime[time_idx]->time);
            float next_time_val = fixTime(parsed_sky.skytime[nexttime_idx]->time);
            assert(time_val != next_time_val);
            if (time_val >= next_time_val)
            {
                if (_time >= time_val || _time < next_time_val)
                {
                    success = true;
                }
            } else if (_time >= time_val && _time < next_time_val)
            {
                success = true;
            }
            if (success)
                break;
        }
        assert(success);
    }
    Parse_SkyTime *early = parsed_sky.skytime[time_idx];
    Parse_SkyTime *late  = parsed_sky.skytime[nexttime_idx];
    if (parsed_sky.sun_time_count <= 1)
    {
        ratio = 1.0;
    } else
    {
        float t_dist = fixTime(_time - early->time);
        float t_len  = fixTime(late->time - early->time);
        ratio        = t_len == 0.0f ? 0.0 : t_dist / t_len;
        ratio        = std::max(0.0f, std::min(1.0f, ratio));
    }
    for (int i = 0; i < parsed_sky.sun_time_count; ++i)
    {
        if (parsed_sky.skytime[i]->sky)
            segs_gfxNodeSetAlpha(parsed_sky.skytime[i]->sky, 0, 1);
    }
    float r = ratio;
    if (early->sky && late->sky && early->sky->model == late->sky->model)
        r = 0.0;
    if (early->sky)
    {
        segs_gfxNodeSetAlpha(early->sky, int32_t((1.0f - r) * 255.0f), 1);
        early->sky->mat.TranslationPart = player_offset + early->skypos;
    }
    if (late->sky)
    {
        segs_gfxNodeSetAlpha(late->sky, int32_t(r * 255.0f), 1);
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
    float rot              = (_time - 12.0f) * glm::two_pi<float>() / 24.0f;
    rot                    = normalizeRadAngle(rot);
    rot                    = rot * std::sqrt(std::fabs(rot)) / std::sqrt(glm::pi<float>());
    for (int i = 0; i < parsed_sky.num_moons; ++i)
    {
        float scale = LinearInterpolateValue(early->moon_scales[i], late->moon_scales[i], ratio);
        if (celestial_objects[i].node)
            fixupCelestialObject(i, 0.0, rot, &player_offset, celestial_objects[i].node, scale);
        if (celestial_objects[i].glow)
            fixupCelestialObject(i, 0.0, rot, &player_offset, celestial_objects[i].glow, scale);
    }

    if (_time <= (*parsed_sky.sky)->LampLightTime.y)
    {
        tmp = std::min(1.0f, _time - ((*parsed_sky.sky)->LampLightTime.x - 1.0f));
    } else
    {
        tmp = std::min((*parsed_sky.sky)->LampLightTime.y + 1.0f - _time, 1.0f);
    }
    g_sun.lamp_alpha = (1.0f - std::max(tmp, 0.0f)) * 255.0f;
    setGlobalShadowColor(&early->shadowcolor, &late->shadowcolor, ratio);
    Vector3 ambient      = LinearInterpolateVectors(early->ambient, late->ambient, ratio);
    Vector3 diffuse      = LinearInterpolateVectors(early->diffuse, late->diffuse, ratio);
    g_sun.ambient.ref3() = (ambient * 63.0) / 255.0f; // basically (ambient/255) * 1/4
    g_sun.ambient.w      = 1.0;
    g_sun.diffuse.ref3() = (diffuse * 63.0) / 255.0f;
    g_sun.diffuse.w      = 1.0;
    if (celestial_objects[0].node)
    {
        g_sun.direction = celestial_objects[0].node->mat.TranslationPart - player_offset;
        g_sun.position  = celestial_objects[0].node->mat.TranslationPart;
    }
    g_sun.direction.ref3().normalize();
    g_sun.direction.y                  = std::abs(g_sun.direction.y);
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

    g_sun.no_angle_light.w           = 1.0;
    g_sun.ambient_for_players.w      = 1.0;
    g_sun.diffuse_for_players.w      = 1.0;
    g_sun.ambient_for_players.ref3() = g_sun.ambient.ref3() * playerAmbientAdjuster;
    g_sun.diffuse_for_players.ref3() = g_sun.diffuse.ref3() * playerDiffuseAdjuster;
    for (int i = 0; i < 3; ++i)
    {
        g_sun.ambient[i]             = std::max(MINIMUM_AMBIENT, g_sun.ambient[i]);
        g_sun.ambient_for_players[i] = std::max(MINIMUM_PLAYER_AMBIENT, g_sun.ambient_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.ambient[i]             = std::min(MAXIMUM_AMBIENT, g_sun.ambient[i]);
        g_sun.ambient_for_players[i] = std::min(MAXIMUM_PLAYER_AMBIENT, g_sun.ambient_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.diffuse[i]             = std::max(MINIMUM_DIFFUSE, g_sun.diffuse[i]);
        g_sun.diffuse_for_players[i] = std::max(MINIMUM_PLAYER_DIFFUSE, g_sun.diffuse_for_players[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        g_sun.diffuse[i]             = std::min(MAXIMUM_DIFFUSE, g_sun.diffuse[i]);
        g_sun.diffuse_for_players[i] = std::min(MAXIMUM_PLAYER_DIFFUSE, g_sun.diffuse_for_players[i]);
    }
    g_sun.no_angle_light.ref3() = g_sun.ambient.ref3() + g_sun.diffuse.ref3();
}
void segs_model_cache_FreeVBO(GeometryData *bf)
{

    if (!bf->vertices && !bf->gl_index_buffer)
        return;
    segs_wcw_statemgmt_FreeVBO(GL_ARRAY_BUFFER, 1, &bf->gl_vertex_buffer, "ModelCache_VBO");
    segs_wcw_statemgmt_FreeVBO(GL_ELEMENT_ARRAY_BUFFER, 1, &bf->gl_index_buffer, "ModelCache_VBO");
    if (bf->cpuside_memory)
        COH_FREE(bf->cpuside_memory);
    bf->gl_index_buffer     = 0;
    bf->triangles           = nullptr;
    bf->normals_offset      = nullptr;
    bf->uv1_offset          = nullptr;
    bf->uv2_offset          = nullptr;
    bf->tangents_directions = nullptr;
    delete bf->segs_data;
    bf->segs_data   = nullptr;
    bf->weights     = nullptr;
    bf->boneIndices = nullptr;
}

void modelFreeCache(Model *model) // fn_4EB440
{
    if (model->loadstate == 4)
    {
        FreeModelGeometry(model);
        segs_model_cache_FreeVBO(model->vbo);
    }
}
static void modelListFree(AnimList *anm_lst)
{
    if (anm_lst->loadstate != 4)
        return;
    for (int i = 0; i < anm_lst->headers->num_subs; ++i)
    {
        Model *model = &anm_lst->headers->models[i];
        modelFreeCache(model);
        COH_FREE(model->trck_node);
        if (model->vbo)
            COH_FREE(model->vbo->textureP_arr);
        delete model->vbo;
    }
    fn_5B8FA0(GeoHashTable, anm_lst->name);
    COH_FREE(anm_lst->header_data);
    COH_FREE(anm_lst->geo_data);
    COH_FREE(anm_lst);
}
void modelFreeAllCache(int unuse_type)
{
    if (!GeoHashTable)
        return;

    Hash_Iter iter;
    hashtab_first(GeoHashTable, &iter);
    for (;;)
    {
        HashTab_Entry *element = hashtab_next(&iter);
        if (!element)
            break;
        AnimList *anm_lst = (AnimList *)HashTable_GetEntryValue(element);
        for (int i = 0; i < anm_lst->headers->num_subs; ++i)
            modelFreeCache(&anm_lst->headers->models[i]);
        anm_lst->geo_use_type &= ~unuse_type;
        if (!anm_lst->geo_use_type)
            modelListFree(anm_lst);
    }
}

void gfxTreeInitCharacterAndFxTree()
{
    if (MP_GfxNode)
        fn_5B7BE0(MP_GfxNode);
    MP_GfxNode = MemPool__Alloc();
    MemPool__InitPool(MP_GfxNode, sizeof(GfxTree_Node), 2000, __FILE__, __LINE__);
    gfx_tree_root      = nullptr;
    gfx_tree_root_tail = nullptr;
    modelFreeAllCache(8); // modelFreeAllCache, 8- GEO_USED_BY_GFXTREE
}
void segs_gfxTreeInit()
{
    gfxTreeInitCharacterAndFxTree();
    gfxTreeInitSkyTree();
}
GfxTree_Node *segs_gfxtree_Allocgfxtreenode(MemPool *pool)
{
    static uint32_t g_gfx_node_counter = 1;
    GfxTree_Node *  node = (GfxTree_Node *)getMemFromPool(pool, __FILE__, __LINE__);
    assert(node);
    node->trick_node = nullptr;
    node->setAlpha(255);
    if (g_gfx_node_counter == 0xFFFFFFFF)
        g_gfx_node_counter += 2;
    node->unique_id = g_gfx_node_counter++;
    return node;
}
GfxTree_Node *segs_createGfxTreeNode_with_parent(GfxTree_Node *parent_node)
{
    GfxTree_Node *curr = segs_gfxtree_Allocgfxtreenode(MP_GfxNode);
    gfxtree_CreateEnqueNode(parent_node, curr, &gfx_tree_root, &gfx_tree_root_tail);
    return curr;
}
void segs_gfxtree_gfxTreeDelete(GfxTree_Node *tree)
{
    if (!tree)
        return;

    gfx_tree_RemoveNodeFromTree(tree, &gfx_tree_root, &gfx_tree_root_tail);
    gfxtree_4ED460(tree->children_list, MP_GfxNode);
    gfxtree_FreeGfxNodeContents(tree, MP_GfxNode);
}
HGLRC segs_createGL()
{
    int pfattribs[] = {WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, WGL_SUPPORT_OPENGL_ARB, GL_TRUE, WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                       WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, WGL_COLOR_BITS_ARB, 32, WGL_DEPTH_BITS_ARB, 24,
                       WGL_STENCIL_BITS_ARB, 8,
                       // WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                       0};
    float                        pf_float_attribs[] = {0};
    int                          attribs[]          = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                     4,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     0,
                     WGL_CONTEXT_PROFILE_MASK_ARB,
                     WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                     WGL_CONTEXT_FLAGS_ARB,
                     WGL_CONTEXT_DEBUG_BIT_ARB,
                     0};
    static PIXELFORMATDESCRIPTOR pfd_old{sizeof(PIXELFORMATDESCRIPTOR),
                                         1,
                                         PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
                                         PFD_TYPE_RGBA,
                                         32,0,0,0,
                                         0,0,0,0,
                                         0,0,0,0,
                                         0,0,24,8,
                                         0,0,0,0,0,0};
    int pixfmt = ChoosePixelFormat(device_context_handle, &pfd_old);
    if (!SetPixelFormat(device_context_handle, pixfmt, &pfd_old))
        return nullptr;
    auto old_ctx = wglCreateContext(device_context_handle);
    if (!old_ctx || !wglMakeCurrent(device_context_handle, old_ctx))
        return nullptr;
    glewExperimental = GL_TRUE;
    glewInit();
    wglMakeCurrent(nullptr, nullptr);
    HGLRC true_ctx = nullptr;
    wglDeleteContext(old_ctx);
    unsigned int numpf;
    if (!wglChoosePixelFormatARB(device_context_handle, pfattribs, pf_float_attribs, 1, &pixfmt, &numpf))
    {
        return nullptr;
    }
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(device_context_handle, pixfmt, sizeof(pfd), &pfd);
    if (SetPixelFormat(device_context_handle, pixfmt, &pfd))
    {
        true_ctx = wglCreateContextAttribsARB(device_context_handle, nullptr, attribs);
        if (true_ctx)
        {
            if (!wglMakeCurrent(device_context_handle, true_ctx))
            {
                GetLastError();
                wglDeleteContext(true_ctx);
                return nullptr;
            }
        }
    }
    segs_createSamplers();
    return true_ctx;
}
void segs_drawBox(int x1, int y1, int x2, int y2, uint32_t argb)
{
    MaterialDefinition flat_material(g_default_mat);
    flat_material.setDrawMode(DrawMode::COLORONLY);
    flat_material.setFragmentMode(eBlendMode::MULTIPLY);
    segs_texSetAllToWhite();
    flat_material.draw_data.projectionMatrix   = glm::ortho(0.0, 640.0, 0.0, 480.0, -1.0, 100.0);
    flat_material.draw_data.modelViewMatrix    = glm::mat4(1);
    flat_material.draw_data.light0.State       = 0;
    flat_material.draw_data.fog_params.enabled = 0;
    flat_material.render_state.setDepthTestMode(RenderState::CMP_NONE);
    flat_material.render_state.setDepthWrite(true);
    flat_material.draw_data.globalColor = {1, 1, 1, 1};

    flat_material.apply();
    drawColoredBox(x1, y1, x2, y2, argb, argb, argb, argb, flat_material);
}

static int cmpSortThingsType(const SortThing *a, const SortThing *b)
{
    int result = a->blendMode - b->blendMode;
    if (result == 0)
        result = (a->model - b->model) / sizeof(Model);
    return result;
}
static int cmpSortThingsDist(const SortThing *a, const SortThing *b)
{
    if (a->distsq == b->distsq)
        return 0;
    if (a->distsq < b->distsq)
        return -1;
    return 1;
}
static void drawSortList(int (*cmpFunc)(const SortThing *, const SortThing *), std::vector<SortThing> &c)
{
    GLDebugGuard debug_guard(__FUNCTION__);
    if (cmpFunc && !(g_State.view.perf_flags & 0x200))
        qsort(c.data(), c.size(), sizeof(SortThing), reinterpret_cast<int (*)(const void *, const void *)>(cmpFunc));
    for (SortThing &thing : c)
    {
        if (thing.modelSource == 1)
        {
            assert(thing.gfxnode);
            segs_modelDrawGfxNode(thing.gfxnode);
        } else
        {
            segs_rendertree_modelDrawWorldmodel(&vsArray[thing.vsIdx]);
        }
    }
}
void drawSortedModels()
{
    GLDebugGuard debug_guard(__FUNCTION__);
    startTypeDrawing = s_blend_mode_switches;
    drawSortList(cmpSortThingsType, ModelArrayTypeSort);
    startAlphaDrawing = s_blend_mode_switches;
    drawSortList(cmpSortThingsDist, ModelArrayDistSort);
    startShadowDrawing = s_blend_mode_switches;
    segs_rendertree_drawShadows();
    endDrawModels = s_blend_mode_switches;
}
static void finishLoadScreen()
{

    while (int32_7B7E24 < int32_6F13A8)
    {
        gfx_48B1B0("blarg", 1000000);
        Sleep(1u);
    }
    DWORD start_count = GetTickCount();
    while (GetTickCount() - start_count < 250)
    {
        uint32_t alpha = 0xFF * (GetTickCount() - start_count) / 250;
        if (alpha > 0xFF)
            alpha = 0xFF;
        showBG(alpha << 24);
        Sleep(4);
    }
    showBG(0xFF000000);
}
void segs_setLightForCharacterEditor()
{
    static float angle        = 0;
    g_sun.ambient_for_players = {0.15f, 0.15f, 0.15f, 1.0f};
    g_sun.diffuse_for_players = {0.19f, 0.19f, 0.19f, 0.19f};
    float x                   = std::cos(angle);
    float y                   = std::sin(angle);
    angle += 0.005f;
    g_sun.direction.x = x; // 1.0;
    g_sun.direction.y = y; // 1.0;
    g_sun.direction.z = 1.0;
}
static void rdrFixMat(Matrix4x3 *to)
{
    Matrix3x3 unitfix;
    unitfix    = {{-1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    to->ref3() = to->ref3() * unitfix;
}
void segs_gfxSetViewMat(const Matrix4x3 *cam_mat, Matrix4x3 *view_mat, Matrix4x3 *inv_viewmat)
{

    *view_mat        = *cam_mat;
    view_mat->ref3() = view_mat->ref3().transpose();
    rdrFixMat(view_mat);
    Vector3 tmp               = -view_mat->TranslationPart;
    view_mat->TranslationPart = view_mat->ref3() * tmp;
    if (inv_viewmat)
        ReverseTransform(view_mat, inv_viewmat);
}
void printGeneralDebug() // 489C00
{
    xyprintf(10, 10, "maxSimultaneousFx %d", maxSimultaneousFx);
    xyprintf(10, 11, "fxCreatedCount %d", fxCreatedCount);
    xyprintf(10, 12, "fxDestroyedCount %d", fxDestroyedCount);
    xyprintf(10, 15, "seqLoadInstCalls %d", seqLoadInstCalls);
    xyprintf(10, 16, "seqFreeInstCalls %d", seqFreeInstCalls);
    xyprintf(10, 18, "drawGfxNodeCalls %d", drawGfxNodeCalls);
    drawGfxNodeCalls = 0;
    xyprintf(10, 19, "drawOrder %d", drawOrder);
    g_nextdraw = 0;
    drawOrder  = 0;
    xyprintf(10, 20, "boned\t\t%d", boned);
    xyprintf(10, 21, "nonboned\t%d", nonboned);
    nonboned = 0;
    boned    = 0;
    // todo: display shader/material changes here ?
    // todo: show per render pass shader/material switch counts sorted/alpha/shadow/etc.
    s_blend_mode_switches = 0;
    endDrawModels         = 0;
    startShadowDrawing    = 0;
    startTypeDrawing      = 0;
    xyprintf(10, 38, "sortedByDist\t\t   %d", sortedByDist);
    xyprintf(10, 39, "sortedByType\t       %d", sortedByType);
    NodesDrawn = 0;
}
static int cmpPtrQSort(const DefTracker **a, const DefTracker **b)
{
    if (*a >= *b)
        return *a != *b;
    return -1;
}
void segs_gfxUpdateFrame(bool force_render_world, bool head_shot)
{
    ModelArrayTypeSort.clear();
    ModelArrayDistSort.clear();
    vsArray.clear();
    nodeTotal = 0;
    segs_renderUtil_ClearGL();
    g_default_mat.render_state = g_render_state.getGlobal();
    if (!head_shot)
        segs_gfxWindowReshape();
    segs_gfxSetViewMat(&cam_info.cammat, &cam_info.viewmat, &cam_info.inv_viewmat);
    if (g_State.view.game_mode == 2)
        segs_setLightForCharacterEditor();

    segs_setSunLight(&cam_info.viewmat);
    g_sun.direction_in_viewspace.ref3() = cam_info.viewmat.ref3() * g_sun.direction.ref3();
    if (g_State.view.debTest3 == 1.0f)
        printGeneralDebug();
    fx_fxRunEngine();
    if (g_State.view.game_mode == 2 || g_State.view.game_mode == 1 || force_render_world) // Render
    {
        splatShadowsDrawn = 0;
        segs_shadowStartScene();
        resetViewspaceMatCount();
        g_fog_state.enabled = true;
        if (g_State.view.bPrintSeqInfo)
            gfx_PrintSeqInfo();
        segs_texSetAllToWhite();
        segs_wcw_UnBindBufferARB();
        visibleTrays.clear();
        see_outside = 1;
        if (g_State.view.bWireframe && !inEditMode())
            xyprintf(10, 9, "TIME: %f", server_visible_state.map_time_of_day);
        if (g_State.view.bWireframe && !inEditMode())
            xyprintf(10, 9, "TIME: %f", server_visible_state.map_time_of_day);
        if (g_State.view.disablesky)
            segs_sunGlareDisable();
        if (!head_shot)
        {
            if (!inEditMode() && !g_State.view.ortho && !g_State.view.disablesky)
                segs_gfxTreeDrawNodeSky(sky_gfx_tree_root, &cam_info.viewmat);
            if (g_State.view.game_mode == 1 || force_render_world)
                segs_groupDrawRefs(cam_info.viewmat);
        }
        qsort(visibleTrays.data(), visibleTrays.size(), sizeof(DefTracker *),
              (int (*)(const void *, const void *))cmpPtrQSort);
        auto treeroot = gfx_tree_root;
        segs_gfxTreeDrawNode(gfx_tree_root, cam_info.viewmat);
        drawSortedModels();
        segs_shadowFinishScene();
    }
    if (g_State.view.bShowDepthComplexity == 1)
    {
        glClear(GL_STENCIL_BUFFER_BIT);
        assert(!"Removed");
    }
    segs_partRunEngine(); // Run and Render Particle Engine
    if (g_State.view.bShowDepthComplexity == 1)
    {
        assert(!"Removed");
        //        glEnable(GL_STENCIL_TEST);
        //        glStencilFunc(GL_ALWAYS, 0x80, 0xFFFFFFFF);
        //        render_state.setStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    if (!head_shot)
    {
        drawStuffOnEntities();
        font_fontRender();
        segs_sunVisible();
        DoorAnimCheckFade(); // DoorAnimCheckFade
        MaterialDefinition debug_draw_material(g_default_mat);
        debug_draw_material.setDrawMode(DrawMode::COLORONLY);
        debug_draw_material.setFragmentMode(eBlendMode::MULTIPLY);
        if (inEditMode() || g_State.see_everything || g_State.view.perf_flags)
            lightLines2(&cam_info.viewmat, debug_draw_material);
        displayEntDebugInfo();
        drawLightDirLines(&cam_info.viewmat, debug_draw_material);
        displaylog_4890F0();
    }
    // WCWClear & showFramerate
    if (server_visible_state.pause)
        xyprintf(1, 5, "Paused");
    gfx_showFramerate();
    if (!head_shot)
    {
        if (g_State.view.bShowPerformSet)
            printGfxSettings();
        statusLineDraw();
        if (g_State.view.game_mode == 3) // showBg - winupdate
        {
            showBgAdd(2);
            showBG(0);
            showBgAdd(-2);
        } else if (glob_have_camera_pos == 2)
        {
            VfPrintfWrapper("starting game\n");
            showBgAdd(2);
            finishLoadScreen();
            showBgAdd(-2);
            startFadeInScreen(1);
            glob_have_camera_pos = 1;
        } else
        {
            SwapBuffers(device_context_handle);
        }
    }
    if (!head_shot && (g_State.g_spin360_speed || g_State.g_BuildCubeMaps == 6))
        renderUtil_rdrPixBufGetOld();
    fxCleanUp();
    assert(size_t(nodeTotal) + vsArray.size() == ModelArrayDistSort.size() + ModelArrayTypeSort.size());
    ModelArrayTypeSort.clear();
    ModelArrayDistSort.clear();
    vsArray.clear();
    nodeTotal = 0;
}
void segs_onPerfTest(int test_num)
{
    static int s_timer_id;
    int        iterations = 0;
    if (!s_timer_id)
        s_timer_id = CohTimer__AllocTimer();
    fn_4C13C0("_hedgeshort_lrghi");
    Model *      model = findNameInLib_P("_hedgeshort_lrghi")->model;
    TextureBind *bind1 = *model->vbo->textureP_arr;
    CohTimer__ResetTimer(s_timer_id);
    MaterialDefinition testmaterial(g_default_mat);
    testmaterial.setDrawMode(DrawMode::SINGLETEX);
    testmaterial.setFragmentMode(eBlendMode::MULTIPLY);
    while (true)
    {
        ++iterations;
        Matrix4x3 tmp_mat1         = Unity_Matrix;
        tmp_mat1.TranslationPart.y = 75.0;
        tmp_mat1.TranslationPart.z = -1000.0;
        segs_modelDraw(model, &tmp_mat1, nullptr, 0xFF, 0, nullptr, testmaterial);
        switch (test_num)
        {
        case 0:
            for (int i = 0; i < 20000; ++i)
            {
                segs_modelDraw(model, &tmp_mat1, nullptr, 0xFF, 0, nullptr, testmaterial);
            }
            break;
        case 1:
            for (int i = 0; i < 20000; ++i)
            {
                model->vbo->draw(*testmaterial.program, GL_TRIANGLES, 3 * model->model_tri_count, 0);
            }
            break;
        case 2:
        {
            int       idx_count = 3 * model->model_tri_count;
            Vector3i *indices   = model->vbo->triangles;
            for (int i = 0; i < 20000; ++i)
            {
                if (idx_count > 0)
                {
                    for (int tri = 0; tri < ((idx_count - 1) / 3 + 1); ++tri)
                    {
                        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices + tri);
                    }
                }
            }
        }
        break;
        case 3:
            for (int i = 0; i < 20000; ++i)
            {
                tmp_mat1.TranslationPart.y += 0.00001f;
                testmaterial.draw_data.modelViewMatrix = tmp_mat1.toGLM();
                glBindTexture(g_whiteTexture->texture_target, g_whiteTexture->gltexture_id);
                glBindTexture(bind1->texture_target, bind1->gltexture_id);
                testmaterial.draw_data.constColor1.w = 1.0;
                testmaterial.updateUniforms();
                glDrawElements(GL_TRIANGLES, 3 * model->model_tri_count, GL_UNSIGNED_INT, model->vbo->triangles);
            }
            break;
        case 31:
            for (int i = 0; i < 20000; ++i)
            {
                tmp_mat1.TranslationPart.y += 0.00001f;
                testmaterial.draw_data.modelViewMatrix = tmp_mat1.toGLM();
                glDrawElements(GL_TRIANGLES, 3 * model->model_tri_count, GL_UNSIGNED_INT, model->vbo->triangles);
            }
            break;
        case 32:
            for (int i = 0; i < 20000; ++i)
            {
                glBindTexture(g_whiteTexture->texture_target, g_whiteTexture->gltexture_id);
                glBindTexture(bind1->texture_target, bind1->gltexture_id);
                glDrawElements(GL_TRIANGLES, 3 * model->model_tri_count, GL_UNSIGNED_INT, model->vbo->triangles);
            }
            break;
        case 33:
            for (int i = 0; i < 20000; ++i)
            {
                testmaterial.draw_data.constColor1.w = 1.0;
                testmaterial.updateUniforms();
                glDrawElements(GL_TRIANGLES, 3 * model->model_tri_count, GL_UNSIGNED_INT, model->vbo->triangles);
            }
            break;
        default: PrintSystemMessage("unknown test_num: %d", test_num); return;
        }
        SwapBuffers(device_context_handle);
        if (CohTimer__elapsed(s_timer_id) >= 2.0f)
        {
            int   tri_count       = 20000 * iterations * model->model_tri_count;
            float seconds_elapsed = CohTimer__elapsed(s_timer_id);
            PrintSystemMessage("test#%d: %fM tris/sec", test_num, double(tri_count / seconds_elapsed / 1000000.0f));
            return;
        }
    }
}
struct SeqGlobals
{
    struct Parser_Sequencer *seq;
    int                      dev_seqInfoCount;
    struct Parser_SeqToken **m_Sequencers;
};
#pragma pack(push, 1)
struct Parser_SeqToken
{
    struct SeqInfo *        info;
    int                     u_0;
    char *                  seq_name;
    struct Parser_TypeDef **_TypeDef;
    int                     _Group;
    int                     _Move;
    int                     a;
    int                     b;
};
#pragma pack(pop)

static_assert(sizeof(CharAttr_Nested1) == 0x14);
extern "C" {
__declspec(dllimport) int ParserLoadFiles(const char *, const char *alternate_ext, const char *, int flags,
                                          CharAttr_Nested1 *loading_template, void *tgt_memory, void *, int *,
                                          signed int (*)(void *, void *));
__declspec(dllimport) void fn_5B37C0(char *, const char *);
__declspec(dllimport) int fn_4F35C0(Parser_SeqToken *, Parser_SeqToken *);
__declspec(dllimport) SeqGlobals seqGlobals;
__declspec(dllimport) CharAttr_Nested1 SeqencerTokens[2];
}

float segs_load_parse_Sequencers()
{

    int now = clock();
    ParserLoadFiles("sequencers", ".txt", "sequencers.bin", 1, SeqencerTokens, &seqGlobals.m_Sequencers, nullptr, nullptr, nullptr);
    size_t sz = COH_ARRAY_SIZE(seqGlobals.m_Sequencers);
    for (int i = 0; i < (signed int)sz; ++i)
    {
        char             buf[260] = {0};
        Parser_SeqToken *tok      = seqGlobals.m_Sequencers[i];
        fn_5B37C0(buf, tok->seq_name);
        strcpy(tok->seq_name, buf);
    }
    qsort(seqGlobals.m_Sequencers, sz, sizeof(Parser_SeqToken *), (int (*)(const void *, const void *))fn_4F35C0);
    return (clock() - now) / 1000.0f;
}
void segs_gfxDoNothingFrame()
{
    fx_fxRunEngine();
    segs_partRunEngine();
    segs_clrLastSpriteIdx();
    Sleep(1000u);
}
void lightLines2(Matrix4x3 *mat, MaterialDefinition &material)
{
    if (line_count)
        drawDebugBeaconsWithMatrix(struct_114C3C0, line_count, mat, material);
    if (beacon_dbg_idx)
        drawDebugBeaconsWithMatrix(int32_114C3AC, beacon_dbg_idx, mat, material);
}
void drawLightDirLines(Matrix4x3 *mat, MaterialDefinition &material)
{
    if (int32_D49520)
        drawDebugBeaconsWithMatrix(struct_D41820, int32_D49520, mat, material);
}
void patch_gfx()
{
    BREAK_FUNC(gfx_4D8580);
    BREAK_FUNC(sunVisible);
    PATCH_FUNC(load_parse_Sequencers);

    PATCH_FUNC(gfx_GfxCardFixes);
    patchit("gfx_48AE20", reinterpret_cast<void *>(segs_drawBox));
    patchit("fn_5810B0", reinterpret_cast<void *>(segs_createGL));
    patchit("gfx_48AE20", reinterpret_cast<void *>(drawFlatBox));
    patchit("gfxtree_4ECE30", reinterpret_cast<void *>(segs_gfxTreeInit));
    patchit("fn_49C760", reinterpret_cast<void *>(resetSunNode));
    patchit("fn_4B0180", reinterpret_cast<void *>(segs_ttDrawBoxBasic));
    patchit("fn_4EB560", reinterpret_cast<void *>(modelFreeAllCache));
    patchit("fn_4EB440", reinterpret_cast<void *>(modelFreeCache));

    PATCH_FUNC(gfxUpdateFrame);
    PATCH_FUNC(setSunLight);
    PATCH_FUNC(sun_SetFog);
    PATCH_FUNC(sun_sunUpdate);
    PATCH_FUNC(onPerfTest);
    PATCH_FUNC(gfxDoNothingFrame);
    PATCH_FUNC(gfxSetViewMat);
    PATCH_FUNC(createGfxTreeNode_with_parent);
    PATCH_FUNC(gfxtree_gfxTreeDelete);
    PATCH_FUNC(gfxNodeSetAlpha);
}
