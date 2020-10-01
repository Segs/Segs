#include "FontManager.h"
#include <GL/glew.h>
#include "utils/helpers.h"
#include <utils/dll_patcher.h>
#include <cstring>
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

struct GenericHashTable;
struct TTFontManager
{
    Array *fonts;
    Array *textures;
    GenericHashTable *glyphCache;
};
struct GenericHashEntry;
extern "C"
{
    __declspec(dllimport) void * fn_5C3500(GenericHashTable *tgt_entry_idx, const char *key_to_find);
    __declspec(dllimport) GenericHashEntry *GenericHashTable_InsertEntry(GenericHashTable *table, char *key, void *value, const char *filename, int line);
    __declspec(dllimport) TTF_Font *fn_4B2440();
    __declspec(dllimport) int ttFontCore_4B24E0(TTF_Font *fn, const char *fontname);
}
TTF_CacheElement * segs_ttFontManager_uploadFontBitmap(TTFontManager *manager, TTF_Font *font, TTFRenderParams *renderParams, ttFontBitmap *bitmap)
{
    TTF_CacheElement key_to_find;

    key_to_find.renderParams = *renderParams;
    GenericHashTable *tab = manager->glyphCache;
    key_to_find.font = font;
    key_to_find.renderParams.font = renderParams->font;
    TTF_CacheElement *entry = (TTF_CacheElement *)fn_5C3500(tab, (char *)&key_to_find);
    if (entry)
    {
        return entry;
    }
    TTF_CacheElement *new_entry = (TTF_CacheElement *)COH_CALLOC(1, sizeof(TTF_CacheElement));
    new_entry->font             = font;
    new_entry->renderParams     = *renderParams;
    new_entry->bitmapInfo       = bitmap->info;
    glGenTextures(1, &new_entry->tex_ids);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, new_entry->tex_ids);
    //TODO: oh gods, make this a texture atlas at least.
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8_EXT, (uint16_t)bitmap->info.tex_dim, (uint16_t)bitmap->info.tex_dim, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, bitmap->bitmap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, (uint16_t)bitmap->info.tex_dim, (uint16_t)bitmap->info.tex_dim, 0, GL_RG,
                 GL_UNSIGNED_BYTE, bitmap->bitmap);
    GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    new_entry->top    = 0.0;
    new_entry->left   = 0.0;
    new_entry->bottom = (float)(uint16_t)bitmap->info.height / (float)(uint16_t)bitmap->info.tex_dim;
    new_entry->right  = (float)(uint16_t)bitmap->info.width / (float)(uint16_t)bitmap->info.tex_dim;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GenericHashTable_InsertEntry(manager->glyphCache, (char *)new_entry, new_entry, __FILE__, __LINE__);
    return new_entry;
}
void segs_freeCacheElement(TTF_CacheElement *elem)
{
    if ( elem->tex_ids )
        glDeleteTextures(1, &elem->tex_ids);
    COH_FREE(elem);
}
TTF_Font *createAndLoadFont(const char *name)
{
    static const std::pair<const char *, const char *> font_replacements[] = {
        {"fonts/redcircl.ttf", "fonts/redcircl.ttf"},
        {"fonts/mont_hvbold.ttf", "fonts/mont_hvbold.ttf"},
        {"fonts/mont_demibold.ttf", "fonts/mont_demibold.ttf"},
        {"fonts/tahomabd.ttf", "fonts/tahomabd.ttf"},
        {"fonts/verdana.ttf", "fonts/verdana.ttf"},
        {"fonts/verdanab.ttf", "fonts/verdanab.ttf"},
        {"fonts/mingliu.ttc", "fonts/mingliu.ttc"},
        {"fonts/gulim.ttc", "fonts/gulim.ttc"},
        {"fonts/cour.ttf", "fonts/cour.ttf"},
        {"fonts/digitalb.ttf", "fonts/digitalb.ttf"},
    };
    for(const auto &entry : font_replacements)
    {
        if(0==strcasecmp(entry.first,name))
        {
            name = entry.second;
            break;
        }
    }
    TTF_Font *fnt = fn_4B2440();
    if ( !ttFontCore_4B24E0(fnt, name) )
    {
        COH_FREE(fnt);
        return nullptr;
    }
    return fnt;
}
void patch_fontmanager()
{
    PATCH_FUNC(ttFontManager_uploadFontBitmap);
    patchit("fn_4B2490",reinterpret_cast<void *>(createAndLoadFont));
    patchit("ttFontManager_4B3940",reinterpret_cast<void *>(segs_freeCacheElement));
}
