#pragma once
#include <stdint.h>
#include "utils/helpers.h"
struct TextureBind
{
    TextureBind *tex_field_0;
    int width;
    int height;
    float u1;
    float u2;
    uint32_t gltexture_id;
    char *name1;
    int flags;
    int load_at_ticks;
    int load_state;
    struct XFileHandle *fh;
    int data_start;
    int src_size;
    const char *tex_directory;
    void *texture_target;
    uint8_t cubemap_face;
    float Gloss;
    int BlendType;
    Vector2 ScaleST0;
    Vector2 ScaleST1;
    struct TexInternalHdr *u4;
    int u5;
    int u6;
    int use_category;
    int u7;
    int byte_size;
    TextureBind *tex_links[3];
};
static_assert (sizeof(TextureBind)==0x7C,"");

void patch_textures();
