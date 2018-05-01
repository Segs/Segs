#include "patch_all_the_things.h"

#include <stdio.h>
#include <windows.h>

HMODULE coh_instance;
extern "C" {
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        coh_instance = GetModuleHandle(nullptr);
        patch_all_the_things();
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
__declspec(dllexport) int test() {
    return 0;
}
}
