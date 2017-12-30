#include "RenderModel.h"

#include "RenderBonedModel.h"
#include "RendererState.h"
#include "Model.h"
#include "utils/dll_patcher.h"
#include "GameState.h"

extern "C" {
    __declspec(dllimport) void xyprintf(int a1, int a2, const char *a3, ...);
    __declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
    __declspec(dllimport) bool inEditMode();

    __declspec(dllimport) int g_nextdraw;
    __declspec(dllimport) int boned;
    __declspec(dllimport) int nonboned;
    __declspec(dllimport) int drawOrder;
}

void segs_modelDrawNode(GfxTree_Node *node)
{
    Model *model = node->model;
    drawOrder = g_nextdraw++ * node->unique_id;
    if (model->Model_flg1 & 0x4000 && node->flg & 0x400000)
    {
        ++boned;
        segs_modelDrawBonedNode(node);
    }
    else
    {
        GlobRenderRel *seq_dat = node->seqGfxData;
        EntLight *light = (seq_dat && (seq_dat->light.use!= ENTLIGHT_DONT_USE)) ? &seq_dat->light : nullptr;

        ++nonboned;
        segs_modelDraw(model, node->viewspace, node->trick_node, node->alpha(), node->rgb_entries, light);
    }
}
void segs_gfxTreeDrawNodeSky(GfxTree_Node *skynode, Matrix4x3 *mat)
{
    Matrix4x3 res;
    int row = 10;
    for (GfxTree_Node *node = skynode; node; node = node->next)
    {
        MatMult4x3(mat, &node->mat, &res);
        Model *model = node->model;
        if (model && node->alpha() >= 5u && model->loadstate & 4 && !(node->flg & 0x80000))
        {
            if (g_State.view.bWireframe && !inEditMode())
                xyprintf(10, row++, "%s %d", model->bone_name_offset, node->alpha());
            segs_modelDraw(model, &res, node->trick_node, node->alpha(), node->rgb_entries, nullptr);
        }
        if (node->children_list)
            segs_gfxTreeDrawNodeSky(node->children_list, &res);
    }
}
void patch_render_node()
{
    PATCH_FUNC(modelDrawNode);
    PATCH_FUNC(gfxTreeDrawNodeSky);
}
