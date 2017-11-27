#include "RendererState.h"
#include "RendererUtils.h"

#include "utils/dll_patcher.h"
#include "GL/glew.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cassert>

struct ClientState
{
    GLenum state_enum;
    int on;
};

extern "C" {
__declspec(dllimport) int enableParticleVBOs;
__declspec(dllimport) int GPU_FLAGS;
__declspec(dllimport) int UsingVBOs;
__declspec(dllimport) int g_using_bump_maps;
__declspec(dllimport) int enableVertShaders;
}
//gl_clientstate
ClientState gl_clientstate[10] = {
    ClientState {GL_VERTEX_ATTRIB_ARRAY1_NV, 0},
    ClientState {GL_VERTEX_ATTRIB_ARRAY5_NV, 0},
    ClientState {GL_VERTEX_ATTRIB_ARRAY6_NV, 0},
    ClientState {GL_VERTEX_ATTRIB_ARRAY7_NV, 0},
    ClientState {GL_TEXTURE_COORD_ARRAY, 0},
    ClientState {GL_TEXTURE_COORD_ARRAY, 0},
    ClientState {GL_VERTEX_ARRAY, 0},
    ClientState {GL_NORMAL_ARRAY, 0},
    ClientState {GL_COLOR_ARRAY, 0},
    ClientState {GL_VERTEX_ATTRIB_ARRAY11_NV, 0},
};
static void enableState(ClientStates state)
{
    assert( GPU_FLAGS & f_GL_VertexShader );
    switch ( state )
    {
    case 0:
        glEnableVertexAttribArrayARB(1);
        return;
    case 1:
        glEnableVertexAttribArrayARB(5);
        return;
    case 2:
        glEnableVertexAttribArrayARB(6);
        return;
    case 3:
        glEnableVertexAttribArrayARB(7);
        return;
    case 4:
        glClientActiveTextureARB(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
        break;
    case 5:
        glClientActiveTextureARB(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
        break;
    case 6:
    case 7:
    case 8:
        glEnableClientState(gl_clientstate[state].state_enum);
        break;
    case 9:
        glEnableVertexAttribArrayARB(11);
        break;
    default:
        return;
    }
}
static void disableState(ClientStates state)
{
    assert( GPU_FLAGS & f_GL_VertexShader );
    switch ( state )
    {
    case 0:
        glDisableVertexAttribArrayARB(1);
        return;
    case 1:
        glDisableVertexAttribArrayARB(5);
        return;
    case 2:
        glDisableVertexAttribArrayARB(6);
        return;
    case 3:
        glDisableVertexAttribArrayARB(7);
        return;
    case 4:
        glClientActiveTextureARB(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
        break;
    case 5:
        glClientActiveTextureARB(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
        break;
    case 6:
    case 7:
    case 8:
        glDisableClientState(gl_clientstate[state].state_enum);
        break;
    case 9:
        glDisableVertexAttribArrayARB(11);
        break;
    default:
        return;
    }
}
void __cdecl segs_enableGlClientState(ClientStates state)
{
    if ( !gl_clientstate[state].on )
    {
        enableState(state);
        gl_clientstate[state].on = 1;
    }
}
void __cdecl segs_disableGlClientState(ClientStates state)
{
    if ( gl_clientstate[state].on == 1 )
    {
        disableState(state);
        gl_clientstate[state].on = 0;
    }
}
void patch_render_state()
{
    PATCH_FUNC(enableGlClientState);
    PATCH_FUNC(disableGlClientState);
}
