#include "EntClient.h"

#include "utils/dll_patcher.h"
#include "renderer/Model.h"
#include "renderer/RenderTricks.h"
#include "renderer/RenderShadow.h"

#include <algorithm>

struct tSplatParams;
extern "C"
{
    __declspec(dllimport) bool gfxTreeNodeIsValid(GfxTree_Node *, int );
    __declspec(dllimport) GfxTree_Node *splat_49A1B0(int *, char *, const char *, int);
    __declspec(dllimport) void gfxTreeFindWorldSpaceMat(Matrix4x3 *, GfxTree_Node *);
    __declspec(dllimport) void updateASplat(tSplatParams *);
}

enum eSplatFlags
{
    SPLAT_ADDITIVE = 0x1,
    SPLAT_SHADOWFADE = 0x2,
    SPLAT_ADDBASE = 0x4,
};

struct tSplatParams
{
    Vector3 projectionStart;
    Vector3 projectionDirection;
    Vector3 scaled_shadow_size;
    float shadowQuality;
    int maxAlpha;
    Matrix4x3 mat;
    uint8_t rgb[3];
    int splatFlags;
    struct Parser_StAnim *stAnim;
    int SplatFalloffType;
    float m_SplatNormalFade;
    int m_SplatFadeCenter;
    GfxTree_Node *simpleshadow;
};
static_assert (sizeof(tSplatParams)==0x78);



void segs_entclient_updateSimpleShadow(SeqInstance *seq, float distFromCamera, float maxDeathAlpha)
{
    static tSplatParams s_SplatParams;
    tSplatParams *splatParams = &s_SplatParams;
    if (!gfxTreeNodeIsValid(seq->simpleShadow, seq->simpleShadowUniqueId))
        seq->simpleShadow = splat_49A1B0(&seq->simpleShadowUniqueId, seq->ent_type_info.ShadowTexture, "white", 0);
    assert(gfxTreeNodeIsValid(seq->simpleShadow, seq->simpleShadowUniqueId));
    splatParams->simpleshadow = seq->simpleShadow;
    splatParams->scaled_shadow_size = seq->ent_type_info.ShadowSize * seq->GeomScale;
    float drawDist = splatParams->scaled_shadow_size.x / 3.6f * 120.0f;
    float distAlpha = 1.0f - (distFromCamera - drawDist) / 40.0f;
    distAlpha = std::max(0.0f,std::min(1.0f,distAlpha));

    uint8_t maxAlpha = seq->seqGfxData.alpha;
    maxAlpha = std::min<uint8_t>(maxAlpha, g_sun.shadowcolor.w * 255.0f);
    maxAlpha = std::min<uint8_t>(maxAlpha, distAlpha * 255.0f);
    maxAlpha = std::min<uint8_t>(maxAlpha, maxDeathAlpha * 255.0f);
    splatParams->maxAlpha = maxAlpha;
    if (seq->gfx_root->children_list)
    {
        Matrix4x3 src;
        gfxTreeFindWorldSpaceMat(&src, seq->gfx_root->children_list);
        float diff = src.TranslationPart.y - seq->gfx_root->mat.TranslationPart.y;
        splatParams->projectionStart = src.TranslationPart;
        splatParams->projectionStart.y -= diff;
    }
    else
    {
        splatParams->projectionStart = seq->gfx_root->mat.TranslationPart;
    }
    Matrix4x3 dest = Unity_Matrix;
    dest.TranslationPart = seq->ent_type_info.ShadowOffset;
    splatParams->mat = seq->gfx_root->mat * dest;
    splatParams->projectionStart += splatParams->mat.ref3() * seq->ent_type_info.ShadowOffset;
    splatParams->projectionStart.y += 2.0;
    segs_shadowStartScene();
    splatParams->projectionDirection = g_sun.shadow_direction;
    splatParams->projectionDirection = { 0, -1, 0 };
    if (seq->ent_type_info.shadowQuality == 1)
        splatParams->shadowQuality = 0.02f;
    if (seq->ent_type_info.shadowQuality == 2)
        splatParams->shadowQuality = 0.2f;
    if (seq->ent_type_info.shadowQuality == 3)
        splatParams->shadowQuality = 0.8f;
    splatParams->rgb[0] = 0;
    splatParams->rgb[1] = 0;
    splatParams->rgb[2] = 0;
    splatParams->SplatFalloffType = 4;
    updateASplat(splatParams);
}
void patch_entclient()
{
    PATCH_FUNC(entclient_updateSimpleShadow);
}
