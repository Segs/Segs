#include "patch_all_the_things.h"

#include <stdio.h>
#include <windows.h>
#include <renderer/ShaderProgramCache.h>

HMODULE coh_instance;
extern "C" {
BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/) {

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        printfDebug("CoH Patch dll loaded\n");
        coh_instance = GetModuleHandle(nullptr);
        patch_all_the_things();
        break;

    case DLL_PROCESS_DETACH:
        printfDebug("Unload working...\n");
        break;

    case DLL_THREAD_ATTACH:
        printfDebug("Thread attach ???\n");
        break;

    case DLL_THREAD_DETACH:
        printfDebug("Thread detach ??\n");
        break;
    }
    return TRUE;
}
__declspec(dllexport) int test() {
    return 0;
}
}
