#pragma once

void patchit(const char *tgt,void *proc);
extern void break_me();
// patch the function named 'name' to call segs_name
#define PATCH_FUNC(name) patchit(#name,(void *)segs_##name)
// make the client assert if the named function is called
#define BREAK_FUNC(name) patchit(#name,(void *)break_me)
