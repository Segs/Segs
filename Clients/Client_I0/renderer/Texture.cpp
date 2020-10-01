#include "Texture.h"

#include "RendererState.h"
#include "RenderSprites.h"
#include "Model.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "GameState.h"

#include "GL/glew.h"
#define STB_IMAGE_IMPLEMENTATION
#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wduplicated-cond"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif
#include "3rd_party/stb/stb_image.h"
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif
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

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
struct HashTable;
struct Parser_Texture
{
    const char *src_file;
    char *      name;
    char *      Blend;
    char *      BumpMap;
    int         u1;
    Vector2     Fade;
    Vector2     ScaleST0;
    Vector2     ScaleST1;
    int         Flags;
    int         BlendType;
    int         u2;
    char *      Surface;
    float       Gloss;
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
int segs_tex_texSwap(TextureBind *tex);
struct ListEntry;
extern "C" {
__declspec(dllimport) int fn_5A0950(const char *fmt, ...);
__declspec(dllimport) Parse_TexSwap ** tex_49F230(int *);
__declspec(dllimport) void remove_from_list(ListEntry *node, ListEntry **head);
__declspec(dllimport) void gfx_48B1B0(const char *, int);
__declspec(dllimport) TextureBind * tex_FindMapEntryTexture(const char *b);
__declspec(dllimport) void * HashTable_queryValue(HashTable *cache_tab, const char *filename);
__declspec(dllimport) int error_ReportParsing(const char *filename, const char *fmt, ...);
__declspec(dllimport) int  AddArrayEntry(void ***arr, void *entry);
__declspec(dllimport) void  tex_4E6300();
__declspec(dllimport) int  tex_4E5CD0(char *name, TextureBind *value);
__declspec(dllimport) bool  file_PathIsDirectory(const char *path);
__declspec(dllimport) int VfPrintfWrapper(const char *fmt,...);
__declspec(dllimport) int  tex_4E6320();
__declspec(dllimport) void fn_4E7FC0();
__declspec(dllimport) int  IsDevelopmentMode();
__declspec(dllimport) int fn_5B4260(struct MemLog *, const char *fmt, ...);
__declspec(dllimport) int jpeg_4924E0(char *, int , TexReadInfo *);
__declspec(dllimport) Parser_Texture *fn_4F4DB0(const char *name);
__declspec(dllimport) int PrintfConsole1(const char *, ...);
__declspec(dllimport) ListEntry * insert_after(ListEntry **, ListEntry *);
__declspec(dllimport) void fn_5B4230(MemLog *);
__declspec(dllimport) XFileHandle *file_find_open(const char *name, const char *mode);
__declspec(dllimport) int file_5A7DE0_fsize(const char *name);
__declspec(dllimport) int XFILE__fseek(XFileHandle *fh, int offset, int origin);
__declspec(dllimport) size_t XFILE__fread(void *buffer, size_t size, size_t count, XFileHandle *fp);
__declspec(dllimport) int fn_5B47F0(const char *, int , int );
__declspec(dllimport) int XFILE__fclose(XFileHandle *fh);
__declspec(dllimport) void utils_report_log_to_file1(const char *fmt, ...);
__declspec(dllimport) void genericlist_5C44B0(ListEntry **, int(*)(const void *, const void *));
__declspec(dllimport) void animMark_asLoaded(AnimList *, int);
__declspec(dllimport) void anim_4EB180(XFileHandle *fp, AnimList *anim_list, int );

__declspec(dllimport) HANDLE background_loader_handle;
__declspec(dllimport) int queuingTexLoadsOutOfThread;
__declspec(dllimport) int g_ThreadedTexLoadDisabled;
__declspec(dllimport) int g_texture_load_mode_isquick;
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) TextureAPC_Arg *TextureGLUploader_getq;
__declspec(dllimport) int int32_85EA64;
__declspec(dllimport) HashTable *g_texbinds_ht;
__declspec(dllimport) TextureBind **g_texbinds;
__declspec(dllimport) TextureBind *g_greyTexture;
__declspec(dllimport) int int32_85EA3C;
__declspec(dllimport) ListEntry *TextureLoaderThread_getq;
__declspec(dllimport) DWORD slowTexLoad;
__declspec(dllimport) int GL_textureMemoryInUse;
__declspec(dllimport) AnimList *globAnimListHead;
__declspec(dllimport) int int32_85EAC0;
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
static volatile int hasStartedQueuing;
static volatile long Addend;
static CRITICAL_SECTION crit_section_8C0;
static CRITICAL_SECTION CriticalSection;
static CRITICAL_SECTION crit_section_8A0;
static CRITICAL_SECTION AnimList_critSection;
static CRITICAL_SECTION CriticalSectionQueueingLoads;

void segs_tex_texSwap2();
void segs_DeleteGLLoadedTexture(TextureBind *tex);

namespace {
struct DDS_PIXELFORMAT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

struct DDS_HEADER
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};
enum {
    DDPF_ALPHAPIXELS = 1,
    DDPF_RGB = 0x40,
};
void loadit(TexReadInfo *tex, XFileHandle *fh)
{
    int mipLevel = 0;
    for (int i = 8 * (tex->format != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) + 8; mipLevel < g_State.view.mipLevel; ++mipLevel)
    {
        if (tex->mip_count <= 0)
            break;
        if (tex->width <= g_State.view.min_tex_size)
            break;
        int fmt = tex->format;
        uint32_t mip_size;
        if (fmt == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT || fmt == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || fmt == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
        {
            mip_size = i * ((tex->width + 3) / 4) * ((tex->height + 3) / 4);
        }
        else
        {
            mip_size = 4 * tex->width * tex->height;
        }
        tex->width /= 2;
        tex->height /= 2;
        if (!tex->height)
            tex->height = 1;
        if (!tex->width)
            tex->width = 1;
        --tex->mip_count;
        tex->size -= mip_size;
        XFILE__fseek(fh, mip_size, 1);
    }
    tex->data = (char *)COH_MALLOC(tex->size);
    XFILE__fread(tex->data, 1, tex->size, fh);
}
#define DDS_DXT1                 0x31545844
#define DDS_DXT3                 0x33545844
#define DDS_DXT5                 0x35545844
void segs_tex_LoadDDS(int byte_count, TexReadInfo *info, XFileHandle *fh)
{
    char buffer[4];
    DDS_HEADER dds_header;
    if (!byte_count)
        return;
    info->data = nullptr;
    info->mip_count = 0;
    info->format = 0;
    info->width = 0;
    info->height = 0;
    info->size = 0;
    XFILE__fread(buffer, 1u, 4, fh);
    if (0 != strncmp(buffer, "DDS ", 4u))
        return;
    XFILE__fread(&dds_header, sizeof(DDS_HEADER), 1, fh);
    info->height = dds_header.dwHeight;
    info->size = byte_count - 0x80;
    info->width = dds_header.dwWidth;
    info->mip_count = dds_header.dwMipMapCount;

    switch (dds_header.ddspf.dwFourCC)
    {
    case DDS_DXT1:
    {
        info->format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        loadit(info, fh);
        return;
    }
    case DDS_DXT3:
    {
        info->format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        loadit(info, fh);
        return;
    }
    case DDS_DXT5:
    {
        info->format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        loadit(info, fh);
        return;
    }
    }

    if (dds_header.ddspf.dwFlags == 0x41)
    {
        if (dds_header.ddspf.dwRGBBitCount == 32)
        {
            if (dds_header.ddspf.dwABitMask == 0xFF000000)
            {
                info->format = 7;
                loadit(info, fh);
                return;
            }
        }
        else if (dds_header.ddspf.dwRGBBitCount == 16)
        {
            if (dds_header.ddspf.dwABitMask == 0x8000)
            {
                info->format = 3;
                loadit(info, fh);
                return;
            }
            if (dds_header.ddspf.dwABitMask == 0xF000)
            {
                info->format = 5;
                loadit(info, fh);
                return;
            }
        }
    }
    else if (dds_header.ddspf.dwFlags == 0x40)
    {
        if (dds_header.ddspf.dwRGBBitCount == DDPF_RGB)
        {
            info->format = 8;
            loadit(info, fh);
            return;
        }
        if (dds_header.ddspf.dwRGBBitCount == 16 && dds_header.ddspf.dwGBitMask == 2016)
        {
            info->format = 4;
            loadit(info, fh);
            return;
        }
    }
    assert(!"cant decode compressed texture");
}
void tex_toPowerOf2_Tex(TexReadInfo *tex)
{
    uint32_t next_w = nextPowerOfTwoLargerThan(tex->width);
    uint32_t next_h = nextPowerOfTwoLargerThan(tex->height);
    if (next_w != tex->width || next_h != tex->height)
    {
        char *data = (char *)COH_CALLOC(next_w * next_h, 4);
        char *data2 = data;
        for(uint32_t row =0; row<tex->height; ++row)
        {
            memcpy(data, tex->data + 4 * row * tex->width, 4 * tex->width);
            data += 4 * next_w;
        }
        COH_FREE(tex->data);
        tex->data = data2;
        tex->width = next_w;
        tex->height = next_h;
        tex->size = 4 * (next_w * next_h);
    }
}
}
void texLoadData(TextureAPC_Arg *arg)
{
    char fname[260] = {0};

    bool file_was_opened = false;
    TexReadInfo *info = &arg->info;

    if (slowTexLoad && !(arg->tex_header.use_category & 1))
        Sleep(slowTexLoad);
    CriticalSectionGuard guard(CriticalSection);

    if (!arg->tex_header.fh)
    {
        sprintf(fname, "texture_library/%s/%s.texture", arg->tex_header.tex_directory, arg->tex_header.name1);
        arg->tex_header.fh = file_find_open(fname, "rb");
        if (!arg->tex_header.fh)
        {
            VfPrintfWrapper("Error!  File 'texture_library/%s/%s.texture' not found when loading texture '%s'!\n", arg->tex_header.tex_directory, arg->tex_header.name1, arg->tex_header.name1);
            return;
        }
        if (g_texture_load_mode_isquick)
            arg->tex_header.src_size = file_5A7DE0_fsize(fname) - arg->tex_header.data_start;
        file_was_opened = 1;
    }
    assert(arg->tex_header.fh);
    XFILE__fseek(arg->tex_header.fh, arg->tex_header.data_start, 0);

    if (arg->tex_header.flags & 0x20)
    {
        assert(!"Refusing to load TGA format texture");
    }
    else
    {
        if ((arg->tex_header.flags & 0x2000) == 0)
        {
            segs_tex_LoadDDS(arg->tex_header.src_size, &arg->info, arg->tex_header.fh);
        }
        else
        {
            std::unique_ptr<uint8_t[]> buf(new uint8_t[arg->tex_header.src_size]);
            XFILE__fread(buf.get(), arg->tex_header.src_size, 1, arg->tex_header.fh);
            int w, h, cn;
            uint8_t *res = stbi_load_from_memory(buf.get(), arg->tex_header.src_size, &w, &h, &cn, 4);
            char *coh_mem_res = (char *)COH_MALLOC(w * h * 4);
            memcpy(coh_mem_res, res, w * h * 4);
            stbi_image_free(res);
            arg->info.width = w;
            arg->info.height = h;
            arg->info.data = coh_mem_res;
            arg->info.format = 7;
            arg->info.size = w * h * 4;
        }
    }
    if (info->data)
    {
        if (arg->tex_header.flags & 0x2000)
            tex_toPowerOf2_Tex(&arg->info);
    }
    arg->tex_header.byte_size = arg->info.size;
    fn_5B47F0("Textures", 1, arg->info.size);
    GL_textureMemoryInUse += arg->tex_header.byte_size;
    if (!info->data)
        VfPrintfWrapper("Warning: Error loading texture_library/%s/%s.texture\n", arg->tex_header.tex_directory, arg->tex_header.name1);
    if (file_was_opened)
    {
        XFILE__fclose(arg->tex_header.fh);
        arg->tex_header.fh = nullptr;
    }
}
static int tex_LoadDetailAndBump(int how, TextureBind *base)
{
    TextureBind *detailtex = base->tex_links[0];
    if (detailtex && detailtex->load_state == 1)
        segs_tex_texLoad2(detailtex->name1, how, base->use_category);
    TextureBind *bumptex = base->tex_links[1];
    if (bumptex && bumptex->load_state == 1)
        segs_tex_texLoad2(bumptex->name1, how, base->use_category);
    return 1;
}
void NTAPI texDoThreadedTextureLoading(TextureAPC_Arg *arg)
{
    if (arg->should_queue == 1 || (arg->should_queue == -1 && int32_85EA3C))
    {
        CriticalSectionGuard guard(crit_section_8C0);
        insert_after(&TextureLoaderThread_getq, (ListEntry *)arg);
    }
    else
    {
        texLoadData(arg);
        if (arg->info.data)
            tex_LoadDetailAndBump(5, &arg->tex_header);
        CriticalSectionGuard guard(crit_section_8C0);
        insert_after((ListEntry **)&TextureGLUploader_getq, (ListEntry *)arg);
    }
    InterlockedDecrement(&Addend);
}
static int texCompareDataStart(const TextureAPC_Arg **a, const TextureAPC_Arg **b)
{
    return (*a)->tex_header.data_start - (*b)->tex_header.data_start;
}
void __stdcall segs_tex_pfnAPC(int)
{
    if ( ++int32_85EA3C == 1 )
    {
        EnterCriticalSection(&CriticalSectionQueueingLoads);
        assert (!hasStartedQueuing );
        hasStartedQueuing = 1;
    }
}
void segs_texDoThreadedQueuedTextureLoading()
{
    TextureAPC_Arg *iter;
    TextureAPC_Arg *next_entry;

    int acc_size = 0;
    utils_report_log_to_file1("loading textures..");
    genericlist_5C44B0(&TextureLoaderThread_getq, (int(*)(const void *, const void *))texCompareDataStart);
    for(iter = (TextureAPC_Arg *)TextureLoaderThread_getq; iter; iter = next_entry)
    {
        next_entry = iter->next;
        remove_from_list((ListEntry *)iter, &TextureLoaderThread_getq);
        texLoadData(iter);
        acc_size += iter->info.size;
        if (iter->info.data)
        {
            for(int i=0; i<2; ++i)
            {
                TextureBind *link = iter->tex_header.tex_links[i];
                if (link && link->load_state == 1)
                    segs_tex_texLoad2(link->name1, 1, iter->tex_header.use_category);
            }
        }
        {
            CriticalSectionGuard guard(crit_section_8C0);
            insert_after((ListEntry **)&TextureGLUploader_getq, (ListEntry *)iter);
        }
        segs_texCheckThreadLoader();
    };
    PrintfConsole1(" (%f Mbytes)", double(float(acc_size) / 1000000));
}
void texLoadQueueStart() //4E65B0
{
    InitializeCriticalSection(&CriticalSectionQueueingLoads);
    hasStartedQueuing = 0;
    QueueUserAPC((PAPCFUNC)segs_tex_pfnAPC, background_loader_handle, 0);
    ++queuingTexLoadsOutOfThread;
}
void __stdcall segs_texLoadQueueFinishDummy(int)
{
    if ( --int32_85EA3C <= 0 )
        LeaveCriticalSection(&CriticalSectionQueueingLoads);
}
void texLoadQueueFinish() // fn_4E7B60
{
    QueueUserAPC((PAPCFUNC)segs_texLoadQueueFinishDummy, background_loader_handle, 0);
    segs_texCheckThreadLoader();
    if ( !--queuingTexLoadsOutOfThread )
    {
        while ( !hasStartedQueuing )
            Sleep(1);
        EnterCriticalSection(&CriticalSectionQueueingLoads);
        LeaveCriticalSection(&CriticalSectionQueueingLoads);
        segs_texDoThreadedQueuedTextureLoading();
        segs_texCheckThreadLoader();
    }
    segs_tex_texSwap2();
}

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

void segs_texCheckThreadLoader()
{
    int loaded_size=0;
    bool tex_uploaded=0;
    TextureBind *binds[10000];

    if (!TextureGLUploader_getq)
        return;

    int texBindsThisTickCount = 0;
    EnterCriticalSection(&crit_section_8C0);
    for(TextureAPC_Arg *apc_arg = TextureGLUploader_getq; apc_arg; apc_arg=TextureGLUploader_getq)
    {
        remove_from_list((ListEntry *)apc_arg, (ListEntry **)&TextureGLUploader_getq);
        TexReadInfo *info = &apc_arg->info;
        if (apc_arg->info.data)
        {
            *apc_arg->targetbind             = apc_arg->tex_header;
            binds[texBindsThisTickCount++] = apc_arg->targetbind;
            assert(texBindsThisTickCount < 10000);
            tex_GL_Upload(&apc_arg->info, apc_arg->targetbind);
            loaded_size += apc_arg->targetbind->src_size;
            tex_uploaded = 1;
        }
        else
        {
            fn_5B4260(nullptr, "%u: BAD TEXTURE %s", g_State.view.tstart_ticks, apc_arg->tex_header.name1);
            VfPrintfWrapper("Missing/bad texture %s \n  (were files deleted while you were running?)\n", apc_arg->tex_header.name1);
            *apc_arg->targetbind = *g_whiteTexture;
        }
        apc_arg->targetbind->load_at_ticks = g_State.view.tstart_ticks;
        apc_arg->targetbind->load_state    = 4;
        if (info->data)
        {
            COH_FREE(info->data);
            info->data = nullptr;
        }
        COH_FREE(apc_arg);
    }
    LeaveCriticalSection(&crit_section_8C0);
    for (int i = 0; i < texBindsThisTickCount; ++i)
        segs_tex_texSwap(binds[i]);
    if (loaded_size)
        gfx_48B1B0("bg_textures", loaded_size);
    if (tex_uploaded)
        fn_4E7FC0();
}
int texBindLowMips(TextureBind *texbind)
{
    TexReadInfo info;

    uint32_t *mip_hdr = (uint32_t *)texbind->mip_data;

    assert(0 == texbind->gltexture_id);
    info.data      = &texbind->mip_data[mip_hdr[0]];
    info.size      = texbind->mip_size - mip_hdr[0];
    info.width     = mip_hdr[1];
    info.height    = mip_hdr[2];
    info.format    = mip_hdr[3];
    int larger_dim = std::max(info.width, info.height);
    int i;
    for (i = 0; larger_dim; ++i)
        larger_dim >>= 1;
    info.mip_count        = i;
    int result            = tex_GL_Upload(&info, texbind);
    texbind->mip_id       = texbind->gltexture_id;
    texbind->gltexture_id = 0;
    return result;
}
int segs_tex_texSwap(TextureBind *tex)
{
    int num_swaps;
    char filename[128];

    Parse_TexSwap **swaps = tex_49F230(&num_swaps);
    int idx = 0;
    if (num_swaps <= 0)
        return -1;
    while (strcasecmp(tex->name1, swaps[idx]->Source))
    {
        if (++idx >= num_swaps)
            return -1;
    }
    if (tex->load_state == 1)
    {
        process_texture_name(swaps[idx]->Dest, filename);
        tex->actualTexture = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
        if (!tex->actualTexture)
        {
            assert(false);
            tex->actualTexture = g_whiteTexture;
            assert(false);
            return idx;
        }
    }
    else
    {
        tex->actualTexture = segs_tex_texLoad2(swaps[idx]->Dest, 1, tex->use_category);
        if (!tex->actualTexture)
        {
            assert(false);
        }
    }
    if (tex->actualTexture == g_whiteTexture)
        assert(false);
    fn_5A0950("Swapping texture '%s' for '%s'\n", swaps[idx]->Source, swaps[idx]->Dest);
    return idx;
}
void segs_tex_texSwap2()
{
    int swap_count;
    std::vector<bool> replaced;
    Parse_TexSwap **swap_arr = tex_49F230(&swap_count);
    if (0!=swap_count)
    {
        replaced.resize(swap_count);
        for(int i=0; i<swap_count; ++i)
        {
            process_texture_name(swap_arr[i]->Source, swap_arr[i]->Source);
            process_texture_name(swap_arr[i]->Dest, swap_arr[i]->Dest);
        }
    }
    int bds = COH_ARRAY_SIZE(g_texbinds);
    for(int i=0; i < bds; ++i)
    {
        g_texbinds[i]->actualTexture = g_texbinds[i];
        int swap_idx = segs_tex_texSwap(g_texbinds[i]);
        if (swap_idx != -1)
            replaced[swap_idx] = 1;
    }
    for (bool was_replaced : replaced)
    {
        if (!was_replaced)
        {
            assert(!"Unable to find source texture to swap");
        }
    }
}
TextureBind *segs_tex_texLoad2(const char *name, int how, int use_category)
{
    int faces;
    TextureBind *res;
    char filename[128];

    faces = 1;
    if (g_ThreadedTexLoadDisabled && how == 1)
        how = 2;
    process_texture_name(name, filename);
    TextureBind *match = (TextureBind *)HashTable_queryValue(g_texbinds_ht, filename);
    if (!match)
        return g_whiteTexture;
    res = match;
    assert(use_category);
    match->use_category = use_category | match->use_category;
    for(int i=0; i<3; ++i)
    {
        if (match->tex_links[i] && !match->tex_links[i]->use_category)
            match->tex_links[i]->use_category = match->use_category;

    }
    if (how == 4)
        return match;
    if (match->flags & 0x200)
    {
        faces = 6;
        match = &match[-match->cubemap_face];
    }
    for(int face=0; face<faces;++face, ++match)
    {
        bool load_state_changed = 0;
        EnterCriticalSection(&crit_section_8C0);
        if (match->load_state == 1)
        {
            load_state_changed = true;
            match->load_state = 2;
        }
        LeaveCriticalSection(&crit_section_8C0);
        if (load_state_changed)
        {
            match->use_category |= use_category;
            TextureAPC_Arg *arg = (TextureAPC_Arg *)COH_CALLOC(sizeof(TextureAPC_Arg), 1);
            fn_5B4230(nullptr);
            arg->targetbind = match;
            arg->tex_header = *match;
            switch(how)
            {
            case 3:
            {
                fn_5B4260(nullptr, "%u: texLoad TEX_LOAD_NOW_CALLED_FROM_LOAD_THREAD %s", g_State.view.tstart_ticks, match->name1);
                arg->should_queue = -1;
                InterlockedIncrement(&Addend);
                texDoThreadedTextureLoading(arg);
                break;
            }
            case 2:
            {
                fn_5B4260(nullptr, "%u: texLoad TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD %s", g_State.view.tstart_ticks, match->name1);
                arg->should_queue = 0;
                InterlockedIncrement(&Addend);
                texDoThreadedTextureLoading(arg);
                segs_texCheckThreadLoader();
                assert(queuingTexLoadsOutOfThread || match->load_state == 4);//TEX_LOADED
                break;
            }
            case 1:
            case 5:
            {
                fn_5B4260(nullptr, "%u: texLoad TEX_LOAD_IN_BACKGROUND %s", g_State.view.tstart_ticks, match->name1);
                assert(background_loader_handle);
                arg->should_queue = -1;
                InterlockedIncrement(&Addend);
                QueueUserAPC((PAPCFUNC)texDoThreadedTextureLoading, background_loader_handle, (DWORD)arg);
                break;
            }
            default:
                assert(false);
            }
        }

        if(!queuingTexLoadsOutOfThread)
        {
            if (match->load_state == 2 && how == 2)
            {
                do
                {
                    if (g_ThreadedTexLoadDisabled)
                        break;
                    Sleep(1);
                    segs_texCheckThreadLoader();
                } while (match->load_state != 4);
            }
        }
    }
    res->load_at_ticks = g_State.view.tstart_ticks;
    return res;
}

static void __stdcall segs_BackgroudLoaderThread()
{
    if (IsDevelopmentMode())
    {
        SetThreadPriority(GetCurrentThread(), -15);
    }
    while (true)
        SleepEx(0xFFFFFFFF, 1);
}
static void NTAPI heyThreadLoadAnAnim(AnimList *anim_list)
{
    CriticalSectionGuard anim_guard(crit_section_8A0);
    if (!anim_list->file)
        anim_list->file = file_find_open(anim_list->name, "rb");
    assert(anim_list->file);
    anim_4EB180(anim_list->file, anim_list, anim_list->type);
    XFILE__fclose(anim_list->file);
    anim_list->file = nullptr;
    {
        CriticalSectionGuard guard(AnimList_critSection);
        insert_after((ListEntry **)&globAnimListHead, (ListEntry *)anim_list);
    }
}
void animRequestForegroundLoad(AnimList *anim_list, XFileHandle *fh) //
{
    anim_list->file = fh;
    anim_list->tex_load_style = 4; //TEX_LOAD_DONT_ACTUALLY_LOAD
    heyThreadLoadAnAnim(anim_list);
    ++int32_85EAC0; //number_of_anims_in_background_loader
}
int  animRequestBackgroundLoad(AnimList *dwData, XFileHandle *fp) //4EB3C0
{
    dwData->file = fp;
    dwData->tex_load_style = 4;
    int result = QueueUserAPC((PAPCFUNC)heyThreadLoadAnAnim, background_loader_handle, (DWORD)dwData);
    ++int32_85EAC0;
    return result;
}
void segs_animCheckThreadLoader()
{
    AnimList *next;
    static int datasize_full;

    if (!globAnimListHead)
        return;

    EnterCriticalSection(&AnimList_critSection);
    for (AnimList *iter = globAnimListHead; iter; iter = next)
    {
        next = iter->next;
        remove_from_list((ListEntry *)iter, (ListEntry **)&globAnimListHead);
        animMark_asLoaded(iter, 4);
        datasize_full += iter->datasize;
        --int32_85EAC0;
    }
    LeaveCriticalSection(&AnimList_critSection);
    if (datasize_full)
        gfx_48B1B0("bg_anims", datasize_full);
    datasize_full = 0;
}
void segs_InitCritSections_BkgLoader()
{
    DWORD ThreadId;
    int Parameter;

    InitializeCriticalSection(&crit_section_8C0);
    InitializeCriticalSection(&AnimList_critSection);
    InitializeCriticalSection(&crit_section_8A0);
    InitializeCriticalSection(&CriticalSection);
    if (!background_loader_handle)
    {
        Parameter = 1;
        background_loader_handle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)segs_BackgroudLoaderThread, &Parameter, 0, &ThreadId);
        assert(background_loader_handle != nullptr);
    }
}
void segs_texBindTexture(GLenum tex_target, int unit, TextureBind *bind)
{

    bind->actualTexture->load_at_ticks = g_State.view.tstart_ticks;
    TextureBind *actual = bind->actualTexture;
    if (bind->actualTexture->load_state == 1)
    {
        if (bind->use_category & 0x11)
            segs_tex_texLoad2(actual->name1, 2, bind->use_category);
        else
            segs_tex_texLoad2(actual->name1, 1, bind->use_category);
    }
    actual = bind->actualTexture;
    if (bind->actualTexture->load_state == 2)
    {
        if (actual->mip_data && !actual->mip_id)
            texBindLowMips(bind->actualTexture);
        if (actual->mip_id)
            segs_wcw_statemgmt_bindTexture(tex_target, unit, actual->mip_id);
        else
            segs_wcw_statemgmt_bindTexture(tex_target, unit, g_greyTexture->gltexture_id);
    }
    else
    {
        segs_wcw_statemgmt_bindTexture(tex_target, unit, actual->gltexture_id);
    }
}
void patch_textures()
{
    patchit("tex_4E6960", reinterpret_cast<void *>(texBindLowMips));
    patchit("tex_4E6960", reinterpret_cast<void *>(texBindLowMips));
    patchit("tex_4E6F40", reinterpret_cast<void *>(segs_texSetBinds));
    patchit("tex_4E6F80", reinterpret_cast<void *>(segs_reloadTextureCallback));
    patchit("fn_4EB3C0", reinterpret_cast<void *>(animRequestBackgroundLoad));
    patchit("anim_4EB040", reinterpret_cast<void *>(animRequestForegroundLoad));
    patchit("tex_4E65B0",reinterpret_cast<void *>(texLoadQueueStart));
    patchit("fn_4E7B60",reinterpret_cast<void *>(texLoadQueueFinish));
    patchit("fn_4E7A60",reinterpret_cast<void *>(segs_texDoThreadedQueuedTextureLoading));
    patchit("tex_4E63C0",reinterpret_cast<void *>(segs_DeleteGLLoadedTexture));
    patchit("fn_4E79C0", reinterpret_cast<void *>(segs_texBindTexture));

    PATCH_FUNC(texCheckThreadLoader);
    PATCH_FUNC(tex_texSwap2);
    PATCH_FUNC(tex_texLoad2);
    PATCH_FUNC(InitCritSections_BkgLoader);
    PATCH_FUNC(animCheckThreadLoader);
}
