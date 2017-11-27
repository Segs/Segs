#include "patch_all_the_things.h"

#include <stdio.h>
#include <windows.h>

HMODULE coh_instance;
extern "C" {
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        printf ("CoH Patch dll loaded\n");
        coh_instance = GetModuleHandle(nullptr);
        patch_all_the_things();
        break;

    case DLL_PROCESS_DETACH:
        printf ("Unload working...\n");
        break;

    case DLL_THREAD_ATTACH:
        printf ("Thread attach ???\n");
        break;

    case DLL_THREAD_DETACH:
        printf ("Thread detach ??\n");
        break;
    }
    return TRUE;
}
__declspec(dllexport) int test() {
    return 0;
}
}
