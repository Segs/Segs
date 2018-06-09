#include "Texture.h"

#include "RendererState.h"
#include "RenderSprites.h"
#include "Model.h"
#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "GameState.h"
#include "GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wduplicated-cond"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include "3rd_party/stb/stb_image.h"
#pragma GCC diagnostic pop
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <vector>
#include <io.h>
#include <windows.h>
#include <synchapi.h>
#include <thread>
#include <algorithm>
#include <memory>

#include <io.h>
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
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
    float Gloss;
};
struct TexReadInfo
{
    char *data;
    int mip_count;
    int format;
    uint32_t width;
    uint32_t height;
    int size;
    bool isS3TCCompressed() const
    {
        return (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
                (format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
                (format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
    }
};
struct TextureAPC_Arg
{
    TextureAPC_Arg *next;
    TextureAPC_Arg *prev;
    TextureBind *   targetbind;
    TextureBind     tex_header;
    TexReadInfo     info;
    int             should_queue;
};
static_assert(sizeof(TextureAPC_Arg) == 0xA4, "");
struct Parse_TexSwap
{
    char *Source;
    char *Dest;
};
struct ListEntry;
extern "C" {
__declspec(dllimport) int fn_5A0950(const char *fmt, ...);
__declspec(dllimport) Parse_TexSwap ** tex_49F230(int *);
__declspec(dllimport) void remove_from_list(ListEntry *node, ListEntry **head);
__declspec(dllimport) void gfx_48B1B0(const char *, int);

__declspec(dllimport) Parser_Texture * fn_4F4DB0(const char *name);
__declspec(dllimport) TextureBind * tex_FindMapEntryTexture(const char *b);
__declspec(dllimport) void * HashTable_queryValue(HashTable *cache_tab, const char *filename);
__declspec(dllimport) int error_ReportParsing(const char *filename, const char *fmt, ...);
__declspec(dllimport) int  AddArrayEntry(void ***arr, void *entry);
__declspec(dllimport) void  tex_4E63C0(TextureBind *tex);
__declspec(dllimport) void  tex_4E6300();
__declspec(dllimport) int  tex_4E5CD0(char *name, TextureBind *value);
__declspec(dllimport) bool  file_PathIsDirectory(const char *path);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int  tex_4E6320();
__declspec(dllimport) int fn_5B47F0(const char *, int , int );
__declspec(dllimport) int XFILE__fclose(XFileHandle *fh);

__declspec(dllimport) HANDLE background_loader_handle;
__declspec(dllimport) int queuingTexLoadsOutOfThread;
__declspec(dllimport) int g_ThreadedTexLoadDisabled;
__declspec(dllimport) int g_texture_load_mode_isquick;
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) TextureAPC_Arg *TextureGLUploader_getq;
__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int int32_85EA64;
__declspec(dllimport) HashTable *g_texbinds_ht;
__declspec(dllimport) TextureBind **g_texbinds;
__declspec(dllimport) TextureBind *g_greyTexture;
__declspec(dllimport) int GL_textureMemoryInUse;
}
struct CriticalSectionGuard
{
    CRITICAL_SECTION &m_section;
    CriticalSectionGuard(CRITICAL_SECTION &s) : m_section(s)
    {
        EnterCriticalSection(&m_section);
    }
    ~CriticalSectionGuard()
    {
        LeaveCriticalSection(&m_section);
    }
};
Parser_Texture *trickFromTextureDirName(const char *dir, const char *name)
{
    char buf[1000] = {0};

    strcat(buf, dir);
    strcat(buf, "/");
    strcat(buf, name);
    return fn_4F4DB0(buf);
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
    const char *name = bind->name1;
    int flags = 0;

    bind->actualTexture = bind;
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
    bind->ScaleST0 = {1.0f,1.0f};
    bind->ScaleST1 = {1.0f,1.0f};
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
    bind->texture_target = (bind->flags & 0x200) != 0 ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    bind->load_state = 1;
}
void  segs_texSetBinds()
{
    int tex_count = COH_ARRAY_SIZE(g_texbinds);
    for (int i = 0; i < tex_count; ++i )
        texSetBindsSub(g_texbinds[i], g_texbinds, i);
}
void segs_DeleteGLLoadedTexture(TextureBind *tex) //4E63C0
{
    if ( tex->load_state != 4 || (signed int)tex->gltexture_id <= 1 )
        return;

    fn_5B47F0("Textures", 3, tex->src_size);
    GL_textureMemoryInUse -= tex->byte_size;
    glDeleteTextures(1, &tex->gltexture_id);
    tex->byte_size = 0;
    tex->load_state = 1;
    tex->gltexture_id = 0;
    if ( tex->fh )
        XFILE__fclose(tex->fh);

    tex->fh = nullptr;
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
                segs_DeleteGLLoadedTexture(found);
            tex_4E6320();
            if ( !tex_4E5CD0(buf2, found) )
                texSetBindsSub(found, nullptr, 0);
            tex_4E6300();
        }
    }
    else
    {
        TextureBind *bind = (TextureBind *)COH_CALLOC(sizeof(TextureBind), 1);
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
static int tex_GL_Upload(TexReadInfo *info, TextureBind *tex)
{
    static std::thread::id s_texLoadThreadId;
    static const GLenum CUBEMAP_FACES_GL[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y };

    int texopt_flags = 0;

    if (s_texLoadThreadId==std::thread::id())
        s_texLoadThreadId = std::this_thread::get_id();
    if (s_texLoadThreadId != std::this_thread::get_id())
    {
        assert(!"texLoad called from wrong thread!");
        return -1;
    }
    TextureBind *tex_bind = tex;
    int height = info->height;
    int width = info->width;
    Parser_Texture *tex_trick = trickFromTextureDirName(tex->tex_directory, tex->name1);
    if (tex_trick)
        texopt_flags = tex_trick->Flags;

    GLenum target = tex->flags & 0x200 ? CUBEMAP_FACES_GL[tex->cubemap_face] : tex->texture_target;
    if (tex->flags & 0x2020)
    {
        tex->flags |= 2;
    }
    else
    {
        switch (info->format)
        {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            tex->flags |= 4;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            tex->flags |= 8;
            break;
        case 7:
        case 8:
            tex->flags |= 2;
            break;
        }
    }
    if (tex->gltexture_id || tex->cubemap_face)
        tex->gltexture_id = tex[-int(tex->cubemap_face)].gltexture_id;
    else
        glGenTextures(1, &tex->gltexture_id);
    segs_wcw_statemgmt_bindTexture(tex->texture_target, 0, tex->gltexture_id);
    assert(!(tex->flags & 0x20)); //tga loading not supported
    if (tex->flags & 0x2000)
    {
        assert(texopt_flags & 0x10000); //TEXOPT_NOMIP
        glTexImage2D(target, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info->data);
    }
    else
    {
        uint32_t mip_level_length;
        int offset = 0;
        int blockSize = 8 * (info->format != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) + 8;
        for(int level =0; level<=info->mip_count; ++level, width >>= 1, height >>= 1)
        {
            if (!width && !height)
                break;
            width = std::max(width, 1);
            height = std::max(height, 1);
            char *data = info->data + offset;
            if (info->isS3TCCompressed())
            {
                uint16_t wx((width + 3) / 4);
                uint16_t hx((height + 3) / 4);
                mip_level_length = blockSize * wx * hx;
                offset += mip_level_length;
                if (offset > info->size)
                {
                    if (!g_texture_load_mode_isquick)
                    {
                        assert(!"Corrupted texture file");
                    }
                    break;
                }
                glCompressedTexImage2DARB(target, level, info->format, width, height, 0, mip_level_length, data);
            }
            else
            {
                assert(!(tex_bind->flags & (4 | 8)));
                mip_level_length = 4 * height * width;
                offset += mip_level_length;
                if (offset > info->size)
                {
                    assert(!"Corrupted texture file");
                }
                // BGRA to RGBA ?
                char *components = data;
                for (int idx = mip_level_length / 4; idx>0; --idx) {
                    std::swap(components[0], components[2]);
                    components += 4;
                }
                glTexImage2D(target, level, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
        }
    }
    int minfilter_linear = 0;
    if (tex_bind->flags & 0x40 && !info->mip_count)
        minfilter_linear = 1;
    else if (texopt_flags & 0x10000)
        minfilter_linear = 1;

    if (tex_bind->flags & 0x80 && !(texopt_flags & 0x2000))
        texopt_flags |= 0x40;
    if (tex_bind->flags & 0x80 && !(texopt_flags & 0x4000))
        texopt_flags |= 0x80;

    GLenum gl_tgt = tex_bind->texture_target;

    if (texopt_flags & 0x40)
    {
        glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
    }
    else if (texopt_flags & 0x200)
    {
        glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    }
    else
    {
        glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
    if ((texopt_flags & 0x80) == 0)
    {
        if (texopt_flags & 0x400)
            glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        else
            glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glTexParameteri(gl_tgt, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
    }
    glTexParameteri(gl_tgt, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(gl_tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (minfilter_linear)
        glTexParameteri(gl_tgt, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (texopt_flags & 0x1000)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    return 1;
}

void patch_textures()
{
    patchit("tex_4E6F40", (void *)segs_texSetBinds);
    patchit("tex_4E6F80", (void *)segs_reloadTextureCallback);
    patchit("tex_4E63C0",reinterpret_cast<void *>(segs_DeleteGLLoadedTexture));

}
