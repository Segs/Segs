#include "patch_all_the_things.h"

#include "entity/entityDebug.h"
#include "renderer/ModelCache.h"
#include "renderer/PBufferRenderer.h"
#include "renderer/RendererUtils.h"
#include "renderer/RenderTricks.h"
#include "renderer/RenderBonedModel.h"
#include "renderer/RenderModel.h"
#include "renderer/RendererState.h"
#include "renderer/RenderShadow.h"
#include "renderer/RenderSprites.h"
#include "renderer/RenderTree.h"
#include "renderer/Texture.h"
#include "renderer/TextureGenerator.h"
#include "graphics/gfx.h"
#include "graphics/SeqGraphics.h"
#include "renderer/RenderParticles.h"
#include "graphics/FontManager.h"
#include "utils/dll_patcher.h"
#include "entity/EntClient.h"
#include "graphics/GroupDraw.h"
int segs_sysutil_IsMemoryDebugEnabled()
{
    return 0;
}
void patch_all_the_things()
{
    PATCH_FUNC(sysutil_IsMemoryDebugEnabled);
    patch_helperutils();
    patch_gfx();
    patch_render_utils();
    patch_rendertricks();
    patch_render_state();
    patch_textures();
    patch_render_model();
    patch_render_node();
    patch_shadow_renderer();
    patch_rendertree();
    patch_ent_debug();
    patch_rendersprites();
    patch_renderparticles();
    patch_pbuffer();
    patch_modelcache();
    patch_fontmanager();
    patch_entclient();
    patch_texgen();
    patch_seqgraphics();
    patch_groupdraw();
    //disable_memMonitor();
}
