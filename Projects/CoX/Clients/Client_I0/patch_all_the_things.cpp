#include "patch_all_the_things.h"
#include "utils/dll_patcher.h"
#include "renderer/RendererUtils.h"
#include "graphics/gfx.h"
void patch_all_the_things()
{
    patch_gfx();
    patch_render_utils();
}
