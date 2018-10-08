#pragma once

#include <QtGlobal>

#ifndef JCON_DLL
#define JCON_DLL
#endif

// Use the generic helper definitions above to define JCON_API, which is used
// for the public API symbols. It either DLL imports or DLL exports (or does
// nothing for static build).

#ifdef JCON_DLL // defined if JCON is compiled as a DLL
    #ifdef jcon_EXPORTS // defined if building the DLL (vs. using it)
        #define JCON_API Q_DECL_EXPORT
    #else
        #define JCON_API Q_DECL_IMPORT
    #endif // jcon_EXPORTS
#else // JCON_DLL is not defined: this means JCON is a static lib.
    #define JCON_API
#endif // JCON_DLL
