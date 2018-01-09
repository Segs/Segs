#include "patch_all_the_things.h"

#include "renderer/RendererUtils.h"
#include "renderer/RenderTricks.h"
#include "renderer/RenderBonedModel.h"
#include "renderer/Texture.h"
#include "renderer/RenderModel.h"
#include "graphics/gfx.h"
#include "renderer/RendererState.h"
#include "renderer/RenderShadow.h"

void patch_all_the_things()
{
    patch_gfx();
    patch_render_utils();
    patch_rendertricks();
    patch_render_state();
    patch_textures();
    patch_render_model();
    patch_render_node();
    patch_shadow_renderer();
}
