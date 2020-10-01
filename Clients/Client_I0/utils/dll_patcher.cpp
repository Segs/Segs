#include "dll_patcher.h"

#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <cassert>
void break_me()
{
    assert(false);
}

extern HMODULE coh_instance;
void patchit(const char *tgt,void *proc)
{
    uint8_t *addr = (uint8_t *)GetProcAddress(coh_instance,tgt);
    DWORD offset = ((DWORD)proc - (DWORD)addr - 5);
    if(addr)
    {
        *addr = 0xE9; // write JMP
        *(DWORD *)(addr+1) = offset; // write target offset
        printf("Patched %s\n",tgt);
    }
    else
    {
        printf("Couldn't patch %s\n",tgt);
        abort();
    }
}
