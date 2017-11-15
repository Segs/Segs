#include "RendererUtils.h"

#include "utils/dll_patcher.h"

#include <stdint.h>
#include <cstring>

extern "C"
{
__declspec(dllimport) int64_t GetCpuMhz();
__declspec(dllimport) int c_assert(const char *,const char *,const char *file,int);
__declspec(dllimport) int renderUtil_GetVersion(char *,const char *driverfile);
__declspec(dllimport) void renderUtil_fn_4DFEF0(char *videocardname,int *pci_ven,int *pci_dev);
__declspec(dllimport) void sysutil_5AFA70(int *total_memory,int *avail_memory);
}
enum GpuVendor
{
    ATI = 0x1002,
    INTEL=0x1086,
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
void segs_renderUtil_GetGfxCardVend(SysInfo_2 *sysinfo)
{
    // fill in current gpu's information
    renderUtil_fn_4DFEF0(sysinfo->video_card, &sysinfo->pci_ven, &sysinfo->pci_dev);
    // fill in memory information
    sysutil_5AFA70(&sysinfo->total_physical_memory, &sysinfo->AvailPhys);
    // fill in cpu
    sysinfo->cpu_freq = GetCpuMhz();
    // TODO: extract card from gl info
    switch(sysinfo->pci_ven)
    {
    case ATI:
    {
        renderUtil_GetVersion(sysinfo->driver_version, "atioglxx.dll");
        break;
    }
    case INTEL:
    {
        //TODO: implement me
        strcpy(sysinfo->driver_version, "Intel v.999");
        break;
    }
    case NVIDIA:
    {
        if ( !renderUtil_GetVersion(sysinfo->driver_version, "nv4_disp.dll") )
            renderUtil_GetVersion(sysinfo->driver_version, "nvdisp.drv");
        break;
    }
    default:
        strcpy(sysinfo->driver_version, "Unknown Vendor");
    }
}

void patch_render_utils()
{
    PATCH_FUNC(renderUtil_GetGfxCardVend);
}
