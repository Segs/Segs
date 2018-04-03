#include "RenderTree.h"

#include "Model.h"
#include "RenderBonedModel.h"
#include "RenderShadow.h"
#include "RenderModel.h"
#include "GameState.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"

extern "C" {
    __declspec(dllimport) int drawGfxNodeCalls;
}

void segs_modelDrawGfxNode(GfxTree_Node *node)
{
    assert(node && node->model);
    assert(node->model->loadstate == 4); // must be loaded ( == 4)
    ++drawGfxNodeCalls;
    if ( g_State.view.bShadowVol != 2 )
    {
        if ( node->seqHandle )
        {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 0x80, 0xFFFFFFFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
    }
    if ( node->m_node_splats )
    {
        if ( node->m_node_splats->drawMe )
        {
            node->m_node_splats->drawMe = 0;
            segs_modelDrawShadowObject(node->viewspace, node->m_node_splats);
        }
    }
    else
    {
        if ( node->assigned_cloth_obj )
        {
            assert(false); // clothing rendering is crazy and/or broken
            //fn_4D6D70(node);
        }
        else
        {
            if ( node->model->Model_flg1 & 0x100 )
                segs_modelDrawAlphaSortHackNode(node);
            else
                segs_modelDrawNode(node);
        }
    }
}

void patch_rendertree()
{
    PATCH_FUNC(modelDrawGfxNode);
}
