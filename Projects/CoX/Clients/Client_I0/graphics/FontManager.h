#pragma once
#include <GL/glew.h>
#include <cstdint>
struct TTF_Font;
struct TTFRenderParams
{
    uint16_t defaultSize;
    uint16_t unicodeCharacter;
    uint16_t renderSize;
    uint16_t align_val;
    union {
        uint32_t flags;
        uint32_t italicize : 1;
        uint32_t bold : 1;
        uint32_t outline : 1;
        uint32_t dropShadow : 1;
        uint32_t softShadow : 1;
    };
    char outlineWidth;
    char dropShadowXOffset;
    char dropShadowYOffset;
    char softShadowSpread;
    unsigned int face;
    struct TTFont *font;
};

static_assert(sizeof(TTFRenderParams) == 0x18, "sizeof(TTFRenderParams)");
struct ttFontBitmapHdr
{
    int16_t width;
    int16_t height;
    int16_t tex_dim;
    int16_t bitmap_top;
    int16_t bitmap_left;
    int16_t glyph_advance;
};
struct ttFontBitmap
{
    ttFontBitmapHdr info;
    uint16_t *bitmap;
};
struct TTF_CacheElement
{
    TTF_Font *font;
    TTFRenderParams renderParams;
    ttFontBitmapHdr bitmapInfo;
    GLuint tex_ids;
    float top;
    float left;
    float right;
    float bottom;
};
static_assert(sizeof(TTF_CacheElement) == 0x3C);
struct TextForEachGlyphParam
{
    int(*render_func)(TextForEachGlyphParam *x);
    TTF_CacheElement *cachedTexture;
    __int16 unicodeCharacter;
    float top;
    float left;
    float bottom;
    float right;
    float nextGlyphLeft;
    int normalized;
};
static_assert(sizeof(TextForEachGlyphParam) == 0x24);

extern void patch_fontmanager();