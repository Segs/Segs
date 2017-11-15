#pragma once
#pragma pack(push, 1)
struct SysInfo_2
{
    int total_physical_memory;
    int AvailPhys;
    char video_card[256];
    int pci_ven;
    int pci_dev;
    float cpu_freq;
    char driver_version[256];
};
#pragma pack(pop)
static_assert(sizeof(SysInfo_2)==0x214,"Struct size must be correct");

extern void segs_renderUtil_GetGfxCardVend(SysInfo_2 *a1);
extern void patch_render_utils();
