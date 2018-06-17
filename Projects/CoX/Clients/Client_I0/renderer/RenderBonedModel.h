#pragma once
#include "utils/helpers.h"
#include "GL/glew.h"
#include <stdint.h>

struct SplatSib;
struct ClothObject;
struct AnimTrack_Entry;
struct TextureBind;
struct TrickNode;
struct GlobRenderRel;
struct Model;
struct MaterialDefinition;

struct RGBA
{
    uint8_t r, g, b, a;
    constexpr RGBA(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
    constexpr RGBA() : r(0), g(0), b(0), a(0) {}
    explicit constexpr RGBA(uint32_t vl) : r((vl >> 24) & 0xFF), g((vl >> 16) & 0xFF), b((vl >> 8) & 0xFF), a(vl & 0xFF)
    {
    }
    Vector3 to3Floats() const { return {r/255.0f,g/255.0f,b/255.0f};}
    Vector4 to4Floats() const { return { r / 255.0f,g / 255.0f,b / 255.0f,a/255.0f }; }
};

struct SplatSib
{
  Vector3 center;
  Vector3 normal;
  Vector3 tangent;
  float height;
  float width;
  float depth;
  TextureBind *texture1;
  TextureBind *texture2;
  int drawMe;
  int flags;
  struct Parser_StAnim *stAnim;
  int falloffType;
  float normalFade;
  int unkn_4C;
  int count_70;
  int unkn_54;
  int vertexCount;
  int triangleCount;
  Vector3 *vertices;
  Vector2 *tex_scroll1;
  Vector2 *tex_scroll2;
  Vector3i *indices; //triangles
  uint32_t *colors; //rgba ?
  SplatSib *invertedSplat;
};
#pragma pack(push, 1)
struct Handle
{
    int16_t  id;
    uint16_t idx;
    operator bool() { return id != 0 || idx != 0; }
    bool operator==(Handle oth) const { return id == oth.id && idx == oth.idx; }
};
#pragma pack(pop)

struct BoneInfo
{
    int numbones;
    int bone_ID[15];
    Vector2 *weights; // never used
    Vector2su *matidxs; // never used
};

#pragma pack(push, 4)
struct GfxTree_Node
{
    Handle           seqHandle;
    char             use_flags;
    GfxTree_Node *   next;
    GfxTree_Node *   children_list;
    int              flg;
    uint8_t          alpha() const { return flg & 0xFF; }
    void             setAlpha(uint8_t v) { flg = (flg & ~0xFF) | v; }
    int              bone_slot;
    Model *          model;
    Matrix4x3 *      viewspace;
    Matrix4x3        mat;
    Vector3          boneScale;
    Vector3          animDelta;
    SplatSib *       m_node_splats;
    ClothObject *    assigned_cloth_obj;
    AnimTrack_Entry *animTrackForAssignedBone;
    char             fx_ref_count;
    GLuint           rgb_entries;
    int              blendMode;
    uint32_t         unique_id;
    GlobRenderRel *  seqGfxData;
    TextureBind *    customtex[2];
    RGBA             color1;
    RGBA             color2;
    TrickNode *      trick_node;
    Model *          shadow_model;
    GfxTree_Node *   parent;
    GfxTree_Node *   prev;
    uint8_t anim_id() const { return bone_slot & 0xFF; }
};
#pragma pack(pop)
static_assert(sizeof(GfxTree_Node) == 0xA8u, "GfxTree_Node");

enum eLightUse : uint8_t
{
    ENTLIGHT_DONT_USE       = 0x0,
    ENTLIGHT_INDOOR_LIGHT   = 0x1,
    ENTLIGHT_PULSING        = 0x2,
    ENTLIGHT_CUSTOM_AMBIENT = 0x4,
    ENTLIGHT_CUSTOM_DIFFUSE = 0x8,
};
struct EntLight
{
    uint8_t use     = ENTLIGHT_DONT_USE;
    uint8_t u_1 = 0; // 1 if already processed by lightEnt
    uint8_t u_2 = 0;
    uint8_t u_3 = 0;
    Vector4 ambient_light;
    Vector4 diffuse;
    Vector4 direction;
    Vector3 tgt_ambient;
    Vector3 tgt_diffuse;
    Vector3 tgt_direction;
    float   interp_rate     = 0;
    float   ambientScale    = 0; // mulitplicative constant
    float   diffuseScale    = 0; // mulitplicative constant
    float   pulseTime       = 0;
    float   pulseEndTime    = 0;
    float   pulsePeakTime   = 0;
    float   pulseBrightness = 0;
    Vector3 calc_pos;
};
static_assert (sizeof(EntLight) == 0x80 );
struct GlobRenderRel
{
    Vector3            boneTranslations[70];
    Matrix4x3          animMatrices[70];
    EntLight           light;
    int                alpha;
    struct DefTracker *tray;
};
extern void segs_wcw_UnBindBufferARB();
extern void segs_modelDraw(Model *model, Matrix4x3 *mat, TrickNode *draw_settings, int alpha, GLuint rgb_entries, EntLight *light_params, const MaterialDefinition &initial_mat);
extern void segs_modelDrawBonedNode(GfxTree_Node *node, const MaterialDefinition &init_mat);
BoneInfo * assignDummyBoneInfo(char *boneset_name);
GLuint segs_modelConvertRgbBuffer(void *data, int count, int own_buffer);

extern void patch_render_model();
