#include "TextureGenerator.h"

#include "Texture.h"
#include "RendererState.h"
#include "utils/dll_patcher.h"
#include "utils/helpers.h"

TextureBind *segs_texGenNew(GLsizei width, GLsizei height)
{

    TextureBind *tex = (TextureBind *)COH_CALLOC(sizeof(TextureBind), 1);
    uint8_t *pixels = new uint8_t[4 * height * width];
    for (int i = 0; i < height * width; ++i )
    {
        pixels[i*4+0] = 0;
        pixels[i*4+1] = 0xFF;
        pixels[i*4+2] = 0;
        pixels[i*4+3] = 0xFF;
    }
    glGenTextures(1, &tex->gltexture_id);
    tex->texture_target = GL_TEXTURE_2D;
    tex->actualTexture = tex;
    tex->load_state = 4;
    segs_wcw_statemgmt_bindTexture(tex->texture_target, 0, tex->gltexture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete [] pixels;
    tex->width = width;
    tex->height = height;
    tex->name1 = "AUTOGEN_TEX";
    tex->tex_directory = "";
    tex->Gloss = 1.0;
    return tex;
}
//sub_4E7DE0
void segs_texGenUpdate(TextureBind *texbind, const GLvoid *pixels)
{
    segs_wcw_statemgmt_bindTexture(texbind->texture_target, 0, texbind->gltexture_id);
    glTexImage2D(texbind->texture_target, 0, GL_RGBA8, texbind->width, texbind->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
void segs_freeGeneratedTexture(TextureBind *tex)
{
    glDeleteTextures(1, &tex->gltexture_id);
    COH_FREE(tex);
}
void patch_texgen()
{
    patchit("tex_gen_4E7BF0",reinterpret_cast<void *>(segs_texGenNew));
    patchit("fn_4E7DB0",reinterpret_cast<void *>(segs_freeGeneratedTexture));
}
