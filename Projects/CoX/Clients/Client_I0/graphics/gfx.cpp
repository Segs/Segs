#include "gfx.h"

#include "utils/dll_patcher.h"
#include "renderer/RendererUtils.h"
#include <stdint.h>
#include <cstring>

extern "C"
{
__declspec(dllimport) int c_assert(const char *,const char *,const char *filename,int line);
}
enum GpuVendor
{
    ATI = 0x1002,
    NVIDIA=0x10DE,
};
enum NvidiaCards
{
    GeForce3 = 0x200,
    Quadro_DCC = 0x203,
};
enum AtiCards
{
    Radeon_9000 = 0x4966,
    Radeon_9500_Pro = 0x4E45,
    Radeon_8500 = 0x514C,
    Radeon_9100 = 0x514D,
};
// should be named: getDefaultPreferences()
void __cdecl segs_gfx_GfxCardFixes(GfxPrefs *settings)
{
    SysInfo_2 sys_info;
    int quality_mod = 3; // original was assuming 0
    //TODO: consider the fact that almost noone has 9500 or GeForce3 anymore
    //TODO: compare lowest end intel graphics card to Radeon 9500 and Geforce3, select default prefs for it
    segs_renderUtil_GetGfxCardVend(&sys_info);
    if ( sys_info.pci_ven == ATI )
    {
        if ( sys_info.pci_dev == Radeon_9500_Pro )
        {
            quality_mod = 3;
        }
        else if ( sys_info.pci_dev == Radeon_9100 ||
                  sys_info.pci_dev == Radeon_9000 ||
                  sys_info.pci_dev == Radeon_8500 )
        {
            quality_mod = 2;
        }
    }
    if ( sys_info.pci_ven == NVIDIA )
    {
        if ( sys_info.pci_dev < Quadro_DCC )
        {
            if ( sys_info.pci_dev < GeForce3 )
                quality_mod = 1;
            else
                quality_mod = 2;
        }
        else
        {
            quality_mod = 3;
        }
    }
    switch(quality_mod)
    {
        case 0:
        case 2:
        settings->mipLevel = 0;
        settings->controls_draw_dist = 1.0;
        settings->LODBias = 1.0;
        settings->gamma = 1.0;
        settings->disableSimpleShadows = 1;
        settings->maxParticles = 50000;
        settings->maxParticleFill_div1mln = 10.0;
        settings->screen_x = 1024;
        settings->screen_y = 768;
        settings->screenX_pos = 0;
        settings->screenY_pos = 0;
        settings->maximized = 0;
        settings->fullscreen = 1;
        settings->fxSoundVolume = 1.0;
        settings->musicSoundVolume = 0.6f;
        settings->enableVBOs = 1;
        break;
    case 1:
        settings->mipLevel = 0;
        settings->controls_draw_dist = 0.7f;
        settings->LODBias = 0.7f;
        settings->gamma = 1.0;
        settings->disableSimpleShadows = 1;
        settings->maxParticles = 50000;
        settings->maxParticleFill_div1mln = 10.0;
        settings->screen_x = 800;
        settings->screen_y = 600;
        settings->screenX_pos = 0;
        settings->screenY_pos = 0;
        settings->maximized = 0;
        settings->fullscreen = 1;
        settings->fxSoundVolume = 1.0;
        settings->musicSoundVolume = 0.6f;
        settings->enableVBOs = 0;
        break;
    case 3:
        settings->mipLevel = 0;
        settings->controls_draw_dist = 1.0;
        settings->LODBias = 1.0;
        settings->gamma = 1.0;
        settings->disableSimpleShadows = 1;
        settings->maxParticles = 50000;
        settings->maxParticleFill_div1mln = 10.0;
        settings->screen_x = 1024;
        settings->screen_y = 768;
        settings->screenX_pos = 0;
        settings->screenY_pos = 0;
        settings->maximized = 0;
        settings->fullscreen = 1;
        settings->fxSoundVolume = 1.0;
        settings->musicSoundVolume = 0.6f;
        settings->enableVBOs = 1;
        break;
    default:
        c_assert("0", "Error getting your gfxsettings.", ".\\graphics\\gfx.c", 1359);
    }
    if ( sys_info.total_physical_memory <= 269000000 )
        settings->mipLevel = 1;
}

void patch_gfx()
{
    PATCH_FUNC(gfx_GfxCardFixes);
}
