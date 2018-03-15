#include "RenderSprites.h"

#include "RendererState.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"

extern "C" {
    __declspec(dllimport) int drawGfxNodeCalls;
    __declspec(dllimport) Vector4 shdr_Constant1;
    __declspec(dllimport) Vector4 shdr_Constant2;
}


void segs_colorsToShaderConstants(uint32_t clr1, uint32_t clr2)
{
    for (int i = 0; i < 4; ++i )
    {
        shdr_Constant1[i] = float((clr1 >> 8 * i) & 0xFF) / 255.0f;
        shdr_Constant2[i] = float((clr2 >> 8 * i) & 0xFF) / 255.0f;
    }
    segs_setShaderConstant(0, &shdr_Constant1);
    segs_setShaderConstant(1, &shdr_Constant2);
}
void patch_rendersprites()
{
    PATCH_FUNC(colorsToShaderConstants);
}
