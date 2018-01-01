#include "Texture.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "GL/glew.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <vector>

#include <io.h>
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
extern "C" {
struct HashTable;
struct Parser_Texture
{
    const char *src_file;
    char *name;
    char *Blend;
    char *BumpMap;
    int u1;
    Vector2 Fade;
    Vector2 ScaleST0;
    Vector2 ScaleST1;
    int Flags;
    int BlendType;
    int u2;
    char *Surface;
    int Gloss;
};
__declspec(dllimport) Parser_Texture * fn_4F4DB0(char *name);
__declspec(dllimport) TextureBind * tex_FindMapEntryTexture(const char *b);
__declspec(dllimport) void * HashTable_queryValue(HashTable *cache_tab, const char *filename);
__declspec(dllimport) int error_ReportParsing(const char *filename, const char *fmt, ...);
__declspec(dllimport) int  AddArrayEntry(void ***arr, void *entry);
__declspec(dllimport) void * dbgCalloc(int strct_size, int count, int blockType, const char *fname, int line);
__declspec(dllimport) void  tex_4E63C0(TextureBind *tex);
__declspec(dllimport) void  tex_4E6300();
__declspec(dllimport) int  tex_4E5CD0(char *name, TextureBind *value);
__declspec(dllimport) bool  file_PathIsDirectory(const char *path);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int  tex_4E6320();

__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int int32_85EA64;
__declspec(dllimport) HashTable *g_texbinds_ht;
__declspec(dllimport) TextureBind **g_texbinds;
__declspec(dllimport) TextureBind *g_greyTexture;
}
int process_texture_name(const char *tex_name, char *name_tgt)
{
    if (tex_name == nullptr || name_tgt == nullptr)
        return 0;
    if (tex_name != name_tgt )
        strcpy(name_tgt, tex_name);
    char *last_dot_ptr = strrchr(name_tgt, '.');
    if (last_dot_ptr == nullptr)
        return 0;
    *last_dot_ptr = 0; // truncate at last dot;
    last_dot_ptr++;
    if (*last_dot_ptr == 0)
        return 0;
    // check extension names
    if (0 == strcmp(last_dot_ptr, "tga"))
        return 0x20;
    if (0 == strcmp(last_dot_ptr, "jpg"))
        return 0x2000;
    if (0 == strcmp(last_dot_ptr, "dds"))
        return 0x40;
    return 0;
}
void texSetBindsSub(TextureBind *bind, TextureBind **binds, int base_idx)
{
    char filename[128];
    char *name = bind->name1;
    int flags = 0;

    bind->tex_field_0 = bind;
    Parser_Texture *tex_info = fn_4F4DB0(name);
    if ( tex_info )
        flags = tex_info->Flags;
    if ( stringContains(bind->tex_directory, "PLAYERS/") || stringContains(bind->tex_directory, "ENEMIES/") || stringContains(bind->tex_directory, "NPCS/") )
        bind->flags |= 0x1080u;
    if ( stringContains(bind->tex_directory, "MAPS/") )
        bind->flags |= 0x80u;
    if ( (flags & 0x8000) != 0 )
    {
        assert(binds); // if binds are null, then this should 'create' them, but original client did not handle that case either.
        for(int face = 0;face<6; ++face)
        {
            binds[base_idx+face]->flags |= 0x280u;
            binds[base_idx+face]->cubemap_face = face;
        }
    }
    if ( tex_info )
        bind->Gloss = tex_info->Gloss;
    if ( flags & 0x800 )
        bind->flags |= 0x400u;
    if ( flags & 0x1000 )
        bind->flags |= 0x800u;
    bind->ScaleST0.x = 1.0;
    bind->ScaleST0.y = 1.0;
    bind->ScaleST1.x = 1.0;
    bind->ScaleST1.y = 1.0;
    if ( tex_info && tex_info->BumpMap )
        bind->tex_links[1] = tex_FindMapEntryTexture(tex_info->BumpMap);
    if ( !(flags & 0x10) )
    {
        if ( !strcasecmp(bind->name1, "invisible") )
        {
            process_texture_name("invisible", filename);
            bind->tex_links[0] = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
        }
        else
        {
            if ( !g_greyTexture )
                g_greyTexture = tex_FindMapEntryTexture("grey");

            bind->tex_links[0] = g_greyTexture;
        }
    }
    else
    {
        process_texture_name(tex_info->Blend, filename);
        bind->tex_links[0] = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
        if ( !bind->tex_links[0])
        {
            error_ReportParsing(tex_info->src_file, "Detail texture %s does not exist for texture trick %s", tex_info->Blend, tex_info->name);
            process_texture_name("grey", filename);
            bind->tex_links[0] = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
        }
        else
        {
            bind->flags |= 0x10u;
            bind->BlendType = tex_info->BlendType;
            bind->ScaleST0 = tex_info->ScaleST0;
            bind->ScaleST1 = tex_info->ScaleST1;
        }
    }
    if ( bind->BlendType == 3 && !strcasecmp(bind->tex_links[0]->name1, "grey") )
    {
        process_texture_name("black", filename);
        bind->tex_links[0] = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
    }
    bind->texture_target = (void *)((bind->flags & 0x200) != 0 ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);
    bind->load_state = 1;
}
void  segs_texSetBinds()
{
    int tex_count = g_texbinds?((int *)g_texbinds)[-2] : 0;
    for (int i = 0; i < tex_count; ++i )
        texSetBindsSub(g_texbinds[i], g_texbinds, i);
}
void  segs_reloadTextureCallback(const char *filename)
{
    char buf1[260];
    char buf2[260];

    if ( strstr(filename, "/_") || file_PathIsDirectory(filename) )
        return;

    strcpy(buf2, filename);
    strcpy(buf1, filename);
    char *tmp_ptr = buf1;
    if ( strrchr(buf1, '/') )
        tmp_ptr = strrchr(buf1, '/') + 1;
    if ( strrchr(tmp_ptr, '.') )
        *strrchr(tmp_ptr, '.') = 0;
    TextureBind *found = tex_FindMapEntryTexture(tmp_ptr);
    if ( found )
    {
        if ( found->load_state == 4 )
        {
            if ( !strcasecmp(found->name1, "white") ||
                 !strcasecmp(found->name1, "grey") ||
                 !strcasecmp(found->name1, "invisible") ||
                 !strcasecmp(found->name1, "black") )
                found->gltexture_id = 0;
            else
                tex_4E63C0(found);
            tex_4E6320();
            if ( !tex_4E5CD0(buf2, found) )
                texSetBindsSub(found, nullptr, 0);
            tex_4E6300();
        }
    }
    else
    {
        TextureBind *bind = (TextureBind *)dbgCalloc(sizeof(TextureBind), 1, 1, __FILE__, __LINE__);
        tex_4E6320();
        if ( !tex_4E5CD0(buf2, bind) )
        {
            if ( !strcasecmp(tmp_ptr, bind->name1) )
            {
                texSetBindsSub(bind, nullptr, 0);
                AddArrayEntry((void ***)&g_texbinds, bind);
            }
            else
            {
                VfPrintfWrapper("File \"%s\" is misnamed, containing texture named \"%s\" - not reloading!\n", filename, bind->name1);
            }
        }
        close(int32_85EA64);
        int32_85EA64 = 0;
    }
}
void patch_textures()
{
    patchit("tex_4E6F40", (void *)segs_texSetBinds);
    patchit("tex_4E6F80", (void *)segs_reloadTextureCallback);
}
