#include "RenderSprites.h"

#include "RendererState.h"
#include  "ShaderProgramCache.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "GameState.h"
#include "Texture.h"
#include "Model.h"
#include "RenderTricks.h"
#include "graphics/FontManager.h"

#include <vector>
#include <numeric>
#include <algorithm>
#include <cstring>
#include "RendererUtils.h"
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

struct Clipper;
struct TTDrawContext;
extern "C" {
    __declspec(dllimport) Clipper *getLastClipper();
    __declspec(dllimport) void get_screen_size(int *, int *);
    __declspec(dllimport) void get_screen_scaling(float *,float *);
    __declspec(dllimport) void  fn_5B6740(float yaw_angle, Matrix3x3 *tgt);
    __declspec(dllimport) void ttTextForEachGlyph(TTDrawContext *draw_ctx, TextForEachGlyphParam *par, float scalex, float scaley, wchar_t *characters_to_render, int textLength);

    __declspec(dllimport) TextureBind *g_whiteTexture;
}
enum eSprites
{
    DISP_SPR_QUAD   = 0,
    DISP_SPR_ROT    = 1,
    DISP_SPR_UV     = 2,
    DISP_SPR_BLEND  = 3,
    DISP_SPR_MIRROR = 4,
    DISP_MOVIE      = 5,
    DISP_STRING     = 6,
};
struct box2d_f
{
    float x;
    float y;
    float w;
    float h;
};
struct Clipper
{
    box2d_f     box;
    box2d_f     gl_box;
    const char *filename;
    int         line;
};

struct DrawTextParam : public TextForEachGlyphParam
{
    float x;
    float y;
    float z;
    RGBA rgba[4];
};
struct SpriteArrayEntry
{
    eSprites     type;
    TextureBind *texture;
    float        xp;
    float        yp;
    float        zp;
    float        xScale;
    float        yScale;
    RGBA         rgba[4];
    uint32_t     argb1;
    uint32_t     argb2;
    int          has_scissor;
    int          additivie;
    float        scisor_left;
    float        scisor_top;
    float        scisor_right;
    float        scisor_bottom;
    float        angle;
};
static_assert(sizeof(SpriteArrayEntry) == 0x50, "sizeof(SpriteArrayEntry)==0x50");
struct TTDrawContext
{
    TTFRenderParams renderParams;
    struct TTF_Font *font;
    int dynamic;
    int dumpDebugInfo;
};
static_assert(sizeof(TTDrawContext) == 0x24, "sizeof(TTDrawContext)");
struct TTBufferredText
{
    float         x;
    float         y;
    float         z;
    float         xScale;
    float         yScale;
    uint32_t      rgba[4];
    wchar_t *     text;
    int           textLength;
    TTDrawContext drawContext;
};
static_assert(sizeof(TTBufferredText)==0x50, "");
static std::vector<SpriteArrayEntry> g_sprites_arr;

void segs_DrawText2DWithScaling(TTDrawContext *draw_ctx, float x, float y, float z, float xScale, float yScale, uint32_t *rgba, wchar_t *text, int len);

unsigned long nextPowerOfTwoLargerThan(unsigned long v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
static bool simpleClippingTest(SpriteArrayEntry *spr, float y_scale, int /*width*/, int height)
{
    if (spr->type == DISP_STRING)
    {
        if (height - spr->scisor_bottom < spr->yp - 40.0f)
            return false;
        if (height - spr->scisor_bottom - y_scale * spr->scisor_top > spr->yp)
            return false;
    }
    else
    {
        if (height - spr->scisor_bottom < y_scale * spr->yp)
            return false;
        if (height - spr->scisor_bottom - y_scale * spr->scisor_top >(spr->texture->height * spr->yScale + spr->yp) * y_scale)
            return false;
    }
    return true;
}
static void clipperInit(SpriteArrayEntry *entry, Clipper *clip)
{
    entry->has_scissor = clip != nullptr;
    if (clip)
    {
        entry->scisor_left = clip->gl_box.x;
        entry->scisor_top = clip->gl_box.h;
        entry->scisor_right = clip->gl_box.w;
        entry->scisor_bottom = clip->gl_box.y;
    }
    else
    {
        entry->scisor_left = 0.0;
        entry->scisor_top = 0.0;
        entry->scisor_right = 1.0;
        entry->scisor_bottom = 1.0;
    }
}
static void segs_addSprite(TextureBind *spr, float xp, float yp, float zp, float xScale, float yScale, uint32_t rgba0,
                           uint32_t rgba1, uint32_t rgba2, uint32_t rgba3, int, int, float angle, int additive,
                           int blend)
{

    if (g_sprites_arr.size() >= 5000)
        return;

    Clipper *clipper = getLastClipper();
    SpriteArrayEntry to_insert;
    to_insert.rgba[0]   = RGBA(rgba0);
    to_insert.rgba[1]   = RGBA(rgba1);
    to_insert.rgba[2]   = RGBA(rgba2);
    to_insert.rgba[3]   = RGBA(rgba3);
    to_insert.additivie = additive;
    if (angle == 0.0f)
    {
        if (blend)
        {
            to_insert.type        = DISP_SPR_BLEND;
            to_insert.argb1       = rgba0;
            to_insert.argb2       = rgba1;
            for (auto &clr : to_insert.rgba)
                clr = RGBA(0xFFFFFFFF);
        }
        else
        {
            to_insert.type = DISP_SPR_QUAD;
        }
    }
    else
    {
        to_insert.type  = DISP_SPR_ROT;
        to_insert.angle = angle;
    }
    to_insert.texture = spr;
    to_insert.xp      = xp;
    to_insert.yp      = yp;
    to_insert.zp      = zp;
    to_insert.xScale  = xScale;
    to_insert.yScale  = yScale;
    clipperInit(&to_insert, clipper);
    g_sprites_arr.emplace_back(to_insert);
}

SpriteArrayEntry *segs_insert_sprite(float depth)
{

    if (g_sprites_arr.size() >= 5000)
        return nullptr;
    g_sprites_arr.emplace_back();
    SpriteArrayEntry *res = &(g_sprites_arr.back());
    res->zp = depth;
    return res;
}
void segs_addSprite_UV_4Color(TextureBind *spr, float xp, float yp, float zp, float xscale, float yscale,
                              uint32_t rgba1, uint32_t rgba2, uint32_t rgba3, uint32_t rgba4, float left, float top,
                              float right, float bottom)
{

    segs_addSprite(spr, xp, yp, zp, xscale, yscale, rgba1, rgba2, rgba3, rgba4, 0.0, 0.0, 0.0, 0, 0);
    SpriteArrayEntry &last(g_sprites_arr.back());
    last.type = DISP_SPR_UV;
    last.scisor_left = left;
    last.scisor_top = top;
    last.scisor_right = right;
    last.scisor_bottom = bottom;
}
void segs_addRotatedSprite(TextureBind *spr, float xp, float yp, float zp, float scaleX, float scaleY, uint32_t clr, float angle, int additive)
{
    segs_addSprite(spr, xp, yp, zp, scaleX, scaleY, clr, clr, clr, clr, spr->hot_x, spr->hot_y, angle, additive, 0);
}
void segs_display_sprite(TextureBind *tex, float x, float y, float z, float scaleX, float scaleY, uint32_t rgba)
{
    if (!tex || !tex->name1)
        return;

    if (scaleX != 0.0f && scaleY != 0.0f)
    {
        segs_addSprite(tex, x, y, z, scaleX, scaleY, rgba, rgba, rgba, rgba, tex->hot_x, tex->hot_y, 0.0, 0, 0);
    }
}
void segs_clrLastSpriteIdx()
{
    g_sprites_arr.clear();
}
extern "C"
{
    __declspec(dllimport) int gCurrentMenu;
    __declspec(dllimport) int show_bg;
}
int loadingScreenVisible() //
{
    return show_bg;
}
bool IsCurrentMenu_E()
{
    return gCurrentMenu < 14;
}
bool inShellMode() //579A60
{
    return IsCurrentMenu_E() || loadingScreenVisible();
}
void segs_drawAllSprites(MaterialDefinition &material)
{
    static std::vector<int> depthSortedSpriteIndices;
    float tex_y_scale=1;
    float tex_x_scale=1;
    int width;
    int height;
    float y_scale=1;
    float x_scale=1;
    int movies_updated = 0;
    int z_adjust = 98;
    GLDebugGuard guard(__FUNCTION__);

    bool in_shell = inShellMode() && !loadingScreenVisible();
    get_screen_size(&width, &height);
    get_screen_scaling(&x_scale, &y_scale);
    auto restore = g_render_state.getGlobal();
    //ShaderProgram &selected = segs_setupShading(DrawMode::SINGLETEX,eBlendMode::MULTIPLY,true);
    if (in_shell)
    {
        if (g_State.view.game_mode == 2)
        {
            guard.insertMessage("ZTest=<=, ZWrite=true");
            material.render_state.setDepthTestMode(RenderState::CMP_LESSEQUAL);
        }
        else
        {
            guard.insertMessage("ZTest=<=, ZWrite=false");
            material.render_state.setDepthWrite(false);
        }
    }
    else
    {
        guard.insertMessage("ZTest=None, ZWrite=true");
        material.render_state.setDepthTestMode(RenderState::CMP_NONE);
        material.render_state.setDepthWrite(true);
    }
    if (in_shell)
    {
        tex_x_scale = x_scale;
        tex_y_scale = y_scale;
    }
    else
    {
        y_scale = 1.0;
        x_scale = 1.0;
    }

    depthSortedSpriteIndices.resize(g_sprites_arr.size());
    std::iota(std::begin(depthSortedSpriteIndices), std::end(depthSortedSpriteIndices), 0); // fill with indices 0-size()-1
    std::stable_sort(std::begin(depthSortedSpriteIndices), std::end(depthSortedSpriteIndices),
        [](int idx1,int idx2)->bool {
        return g_sprites_arr[idx1].zp < g_sprites_arr[idx2].zp;
    });
    float prev_depth = -44400;
    for (int idx : depthSortedSpriteIndices)
    {
        SpriteArrayEntry &sprite_array_entry(g_sprites_arr[idx]);
        assert(sprite_array_entry.zp >= prev_depth);
        prev_depth = sprite_array_entry.zp;

    }
    assert(depthSortedSpriteIndices.size() == g_sprites_arr.size());
    static  GeometryData fakevbo;
    fakevbo.createVAO();
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};
    fakevbo.uploadIndicesToBuffer(indices, 6);
    for (int idx : depthSortedSpriteIndices)
    {
        float bottom = height;
        SpriteArrayEntry &sprite_array_entry(g_sprites_arr[idx]);
        float z = sprite_array_entry.zp / 100.0f;
        if (z > 0.15f && !movies_updated)
        {
            movies_updated = 1;
            // original code has no movie playing in I0
        }
        z -= z_adjust;
        if (sprite_array_entry.has_scissor)
        {
            if (!simpleClippingTest(&sprite_array_entry, y_scale, width, height))
                continue;
            if (in_shell)
            {
                glScissor(x_scale * sprite_array_entry.scisor_left,
                    sprite_array_entry.scisor_bottom,
                    x_scale * sprite_array_entry.scisor_right,
                    y_scale * sprite_array_entry.scisor_top);
            }
            else
            {
                glScissor(sprite_array_entry.scisor_left,
                    sprite_array_entry.scisor_bottom,
                    sprite_array_entry.scisor_right,
                    sprite_array_entry.scisor_top);
            }
            glEnable(GL_SCISSOR_TEST);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
        if (sprite_array_entry.type == DISP_STRING)
        {
            guard.insertMessage("DISP_STRING");
            TTBufferredText *text = (TTBufferredText *)sprite_array_entry.texture;
            float y = bottom - text->y;
            material.apply();
            segs_DrawText2DWithScaling(&text->drawContext, text->x, y, z, text->xScale, text->yScale, text->rgba, text->text, text->textLength);
            continue;
        }
        if (sprite_array_entry.additivie)
            material.render_state.setBlendMode(RenderState::BLEND_ADDALPHA);
        if (sprite_array_entry.type == DISP_SPR_ROT)
        {
            guard.insertMessage("DISP_SPR_ROT");
            float z1 = -1.001f;
            float s = 0.0;
            float t = 0.0;
            float factor_w = float(sprite_array_entry.texture->width) / nextPowerOfTwoLargerThan(sprite_array_entry.texture->width);
            float factor_h = float(sprite_array_entry.texture->height) / nextPowerOfTwoLargerThan(sprite_array_entry.texture->height);

            //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            Matrix4x3 dest = Unity_Matrix;
            fn_5B6740(sprite_array_entry.angle, &dest.ref3());
            float x1 = (float)(-sprite_array_entry.texture->width / 2) * sprite_array_entry.xScale * tex_x_scale;
            float y1 = (float)(sprite_array_entry.texture->height / 2) * sprite_array_entry.yScale * tex_y_scale;
            float x2 = (float)(sprite_array_entry.texture->width / 2) * sprite_array_entry.xScale * tex_x_scale;
            float y2 = (float)(-sprite_array_entry.texture->height / 2) * sprite_array_entry.yScale * tex_y_scale;
            Vector3 offset = Vector3{
                             tex_x_scale * sprite_array_entry.xp + (float)(sprite_array_entry.texture->width / 2) * sprite_array_entry.xScale * tex_x_scale,
                    0,
                    bottom - tex_y_scale * sprite_array_entry.yp - (float)(sprite_array_entry.texture->height / 2) * sprite_array_entry.yScale * tex_y_scale };
            Vector3 pt1 = dest * Vector3{ x1,0,y1 } +offset;
            Vector3 pt2 = dest * Vector3{ x2,0,y1 } +offset;
            Vector3 pt3 = dest * Vector3{ x2,0,y2 } +offset;
            Vector3 pt4 = dest * Vector3{ x1,0,y2 } +offset;
            segs_texBindTexture(GL_TEXTURE_2D, 0, sprite_array_entry.texture);
            material.draw_data.tex_id_0 = sprite_array_entry.texture;
            float vbo_data[] = {
                //positions
                 pt1.x, pt1.z, z1 ,
                 pt2.x, pt2.z, z1 ,
                 pt3.x, pt3.z, z1 ,
                 pt4.x, pt4.z, z1 ,
                 // uvs
                 s, t,
                 factor_w, t ,
                 factor_w, factor_h ,
                 s, factor_h ,
            };
            uint8_t colors[] = {
                sprite_array_entry.rgba[0].r, sprite_array_entry.rgba[0].g, sprite_array_entry.rgba[0].b, sprite_array_entry.rgba[0].a,
                sprite_array_entry.rgba[1].r, sprite_array_entry.rgba[1].g, sprite_array_entry.rgba[1].b, sprite_array_entry.rgba[1].a,
                sprite_array_entry.rgba[2].r, sprite_array_entry.rgba[2].g, sprite_array_entry.rgba[2].b, sprite_array_entry.rgba[2].a,
                sprite_array_entry.rgba[3].r, sprite_array_entry.rgba[3].g, sprite_array_entry.rgba[3].b, sprite_array_entry.rgba[3].a
            };
            fakevbo.uploadVerticesToBuffer(vbo_data, 20);
            fakevbo.uv1_offset = (Vector2 *)(sizeof(Vector3)*4);
            fakevbo.uploadColorsToBuffer(colors,16);
            material.apply();
            fakevbo.draw(*material.program, GL_TRIANGLES, 6, 0);
        }
        else if (sprite_array_entry.type == DISP_SPR_QUAD || sprite_array_entry.type == DISP_SPR_UV || sprite_array_entry.type == DISP_SPR_BLEND)
        {
            assert(sprite_array_entry.type != DISP_SPR_BLEND); // not used in the client code
            float u1 = 0.0f;
            float v1 = 0.0f;
            float u2 = float(sprite_array_entry.texture->width) / (float)nextPowerOfTwoLargerThan(sprite_array_entry.texture->width);
            float v2 = float(sprite_array_entry.texture->height) / (float)nextPowerOfTwoLargerThan(sprite_array_entry.texture->height);
            float wd2 = float(sprite_array_entry.texture->width) * sprite_array_entry.xScale;
            float ht2 = float(sprite_array_entry.texture->height) * sprite_array_entry.yScale;
            if (sprite_array_entry.type == DISP_SPR_UV)
            {
                u1 = sprite_array_entry.scisor_left;
                v1 = sprite_array_entry.scisor_top;
                u2 = sprite_array_entry.scisor_right;
                v2 = sprite_array_entry.scisor_bottom;
                wd2 = nextPowerOfTwoLargerThan(sprite_array_entry.texture->width) * sprite_array_entry.xScale;
                ht2 = nextPowerOfTwoLargerThan(sprite_array_entry.texture->height) * sprite_array_entry.yScale;
            }
            else
            {
                //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                //glSamplerParameteri(perUnitSamplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            segs_texBindTexture(GL_TEXTURE_2D, 0, sprite_array_entry.texture);
            material.draw_data.tex_id_0 = sprite_array_entry.texture;
            float vbo_data[] = {
                 tex_x_scale * sprite_array_entry.xp, bottom - tex_y_scale * sprite_array_entry.yp, z ,
                 tex_x_scale * sprite_array_entry.xp + wd2 * x_scale, bottom - tex_y_scale * sprite_array_entry.yp, z ,
                 tex_x_scale * sprite_array_entry.xp + wd2 * x_scale, bottom - (tex_y_scale * sprite_array_entry.yp + ht2 * y_scale), z ,
                 tex_x_scale * sprite_array_entry.xp, bottom - (tex_y_scale * sprite_array_entry.yp + ht2 * y_scale), z,
                 u1, v1 ,
                 u2, v1 ,
                 u2, v2 ,
                 u1, v2 ,
            };
            uint8_t colors[] = {
                sprite_array_entry.rgba[0].r, sprite_array_entry.rgba[0].g, sprite_array_entry.rgba[0].b, sprite_array_entry.rgba[0].a,
                sprite_array_entry.rgba[1].r, sprite_array_entry.rgba[1].g, sprite_array_entry.rgba[1].b, sprite_array_entry.rgba[1].a,
                sprite_array_entry.rgba[2].r, sprite_array_entry.rgba[2].g, sprite_array_entry.rgba[2].b, sprite_array_entry.rgba[2].a,
                sprite_array_entry.rgba[3].r, sprite_array_entry.rgba[3].g, sprite_array_entry.rgba[3].b, sprite_array_entry.rgba[3].a
            };
            fakevbo.uploadVerticesToBuffer(vbo_data, 20);
            fakevbo.uv1_offset = (Vector2 *)(sizeof(Vector3) * 4);
            fakevbo.uploadColorsToBuffer(colors, 16);
            material.apply();
            fakevbo.draw(*material.program, GL_TRIANGLES, 6, 0);
        }
        if (sprite_array_entry.additivie)
            material.render_state.setBlendMode(RenderState::BLEND_ALPHA);
    }
    segs_clrLastSpriteIdx();
    glDisable(GL_SCISSOR_TEST);
    g_render_state.apply(restore);
}
static int DrawText2DWithScalingHandler(DrawTextParam *param)
{
    TTF_CacheElement *ce     = param->cachedTexture;
    float             top    = param->top + param->y;
    float             left   = param->left + param->x;
    float             bottom = param->bottom + param->y;
    float             right  = param->right + param->x;
    segs_wcw_statemgmt_bindTexture(GL_TEXTURE_2D, 0, ce->tex_ids);
    float vbo_data[] = {
         left, bottom, param->z,
         left, top, param->z ,
         right, top, param->z ,
         right, bottom, param->z ,
         ce->left, ce->bottom,
         ce->left, ce->top ,
         ce->right, ce->top ,
         ce->right, ce->bottom
    };
    uint8_t colors[] = {
        param->rgba[0].r, param->rgba[0].g, param->rgba[0].b, param->rgba[0].a,
        param->rgba[1].r, param->rgba[1].g, param->rgba[1].b, param->rgba[1].a,
        param->rgba[2].r, param->rgba[2].g, param->rgba[2].b, param->rgba[2].a,
        param->rgba[3].r, param->rgba[3].g, param->rgba[3].b, param->rgba[3].a
    };
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    static GeometryData fakevbo;
    fakevbo.createVAO();
    fakevbo.uploadVerticesToBuffer(vbo_data,3*4+2*4);
    fakevbo.uploadIndicesToBuffer(indices, 6);
    fakevbo.uv1_offset = (Vector2 *)(sizeof(Vector3) * 4);

    fakevbo.uploadColorsToBuffer(colors,16);
    fakevbo.draw(*MaterialDefinition::last_applied->program, GL_TRIANGLES, 6, 0);
    return 1;
}
void segs_DrawText2DWithScaling(TTDrawContext *draw_ctx, float x, float y, float z, float xScale, float yScale, uint32_t *rgba, wchar_t *text, int len)
{
    DrawTextParam par;

    par.render_func = (int(*)(TextForEachGlyphParam *))DrawText2DWithScalingHandler;
    if (draw_ctx->dynamic)
    {
        par.x = x;
        par.y = y;
    }
    else
    {
        par.x = std::ceil(x);
        par.y = std::ceil(y);
    }
    par.z = z;
    for(int i=0; i<4; ++i)
        par.rgba[i] = RGBA(rgba[i]);
    ttTextForEachGlyph(draw_ctx, &par, xScale, yScale, text, len);
}
void patch_rendersprites()
{
    g_sprites_arr.reserve(5000);

    patchit("fn_4B33A0", reinterpret_cast<void *>(segs_DrawText2DWithScaling));
    patchit("fn_579580", reinterpret_cast<void *>(segs_addSprite_UV_4Color));
    patchit("fn_579430", reinterpret_cast<void *>(segs_addRotatedSprite));
    patchit("fn_489730", reinterpret_cast<void *>(loadingScreenVisible));
    patchit("fn_579A60", reinterpret_cast<void *>(inShellMode));

    PATCH_FUNC(clrLastSpriteIdx);
    PATCH_FUNC(insert_sprite);
    PATCH_FUNC(display_sprite);
    BREAK_FUNC(colorsToShaderConstants);
    BREAK_FUNC(addSprite);
    BREAK_FUNC(wcwMgmt_EnableFog);
    BREAK_FUNC(fn_579150);
    BREAK_FUNC(fn_578F70);
}
