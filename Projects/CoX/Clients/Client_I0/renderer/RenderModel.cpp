#include "RenderModel.h"

#include "Model.h"
#include "RenderBonedModel.h"
#include "RendererState.h"
// only to retrieve node info map
#include "GameState.h"
#include "RenderTree.h"
#include "RenderTricks.h"
#include "RendererUtils.h"
#include "entity/EntClient.h"
#include "graphics/gfx.h"
#include "utils/dll_patcher.h"

#include <algorithm>

extern "C" {
__declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);
__declspec(dllimport) void MatMult4x3(const Matrix4x3 *self, const Matrix4x3 *oth, Matrix4x3 *res);
__declspec(dllimport) bool inEditMode();
}
int  g_nextdraw;
int  drawOrder;
int  nonboned;
int  boned;
void segs_modelDrawAlphaSortHackNode(GfxTree_Node *node)
{
    float              ref_alpha = std::max(0.0f, float(node->alpha() / 255.0) - 0.4f);
    MaterialDefinition alpha_hack_material(g_default_mat);
    alpha_hack_material.setDrawMode(DrawMode::SINGLETEX);
    alpha_hack_material.setFragmentMode(eBlendMode::MULTIPLY);
    alpha_hack_material.set_useAlphaDiscard(true);
    alpha_hack_material.draw_data.setDiscardAlpha(ref_alpha);
    segs_modelDrawNode(node, alpha_hack_material);
}
void segs_modelDrawNode(GfxTree_Node *node, const MaterialDefinition &initial_mat)
{
    GLDebugGuard debug_guard(__FUNCTION__);
    Model *      model = node->model;
    drawOrder          = g_nextdraw++ * node->unique_id;
    if (model->Model_flg1 & OBJ_DRAW_AS_ENT && node->flg & 0x400000)
    {
        ++boned;
        segs_modelDrawBonedNode(node, initial_mat);
    } else
    {
        GlobRenderRel *seq_dat = node->seqGfxData;
        EntLight *     light   = (seq_dat && (seq_dat->light.use != ENTLIGHT_DONT_USE)) ? &seq_dat->light : nullptr;

        ++nonboned;
        segs_modelDraw(model, node->viewspace, node->trick_node, node->alpha(), node->rgb_entries, light, initial_mat);
    }
}
void segs_modelDrawAlphaSortHack(Model *model, Matrix4x3 *mat, int alpha, Vector3 *mid, GLuint rgbs, EntLight *light)
{
    GLDebugGuard       debug_guard(__FUNCTION__);
    MaterialDefinition mat_def(g_default_mat);
    mat_def.setDrawMode(DrawMode::SINGLETEX);
    mat_def.setFragmentMode(eBlendMode::MULTIPLY);
    mat_def.render_state.stencil_mode = RenderState::STENCIL_ALWAYS;
    mat_def.render_state.stencil_ref  = 64;
    mat_def.render_state.setStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    mat_def.set_useAlphaDiscard(1);
    float mid_len = mid->length();
    if (!g_State.view.fancytrees && mid_len < 50.0f)
    {
        float refb = std::max(0.01f, (mid_len - 35.0f) / 15.0f);
        mat_def.draw_data.setDiscardAlpha(refb);
        mat_def.render_state.setDepthWrite(false);
        alpha = 255;
        segs_modelDraw(model, mat, nullptr, 0xFF, rgbs, light, mat_def);
    }
    float refc = std::max(0.0f, float(alpha) / 255.0f - 0.4f);
    if (g_State.view.fancytrees)
    {

        mat_def.render_state.stencil_mode = RenderState::STENCIL_NONE;
        mat_def.render_state.setDepthWrite(false);
        mat_def.set_useAlphaDiscard(2);
        mat_def.draw_data.setDiscardAlpha(refc);
        segs_modelDraw(model, mat, nullptr, alpha, rgbs, light, mat_def);
        // re-enable stencil testing
        mat_def.render_state.stencil_mode = RenderState::STENCIL_ALWAYS;
    }
    mat_def.draw_data.setDiscardAlpha(refc);
    mat_def.render_state.setDepthWrite(true);
    segs_modelDraw(model, mat, nullptr, alpha, rgbs, light, mat_def);
    mat_def.set_useAlphaDiscard(false);
    // assert(false); // the code below sets up rendering for.... what part exactly ?

    RenderState setup_rs  = g_render_state.getGlobal();
    setup_rs.stencil_mode = RenderState::STENCIL_ALWAYS;
    setup_rs.stencil_ref  = 128;
    setup_rs.stencil_mask = ~0U;
    setup_rs.setStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    g_render_state.apply(setup_rs);
}
void segs_gfxTreeDrawNodeSky(GfxTree_Node *skynode, Matrix4x3 *mat)
{
    Matrix4x3          res;
    int                row = 10;
    MaterialDefinition base_sky_material(g_default_mat);
    base_sky_material.setDrawMode(DrawMode::SINGLETEX);
    base_sky_material.setFragmentMode(eBlendMode::MULTIPLY);
    for (GfxTree_Node *node = skynode; node; node = node->next)
    {
        assert(g_render_state.getGlobal().depth_cmp != RenderState::CMP_NONE);
        MatMult4x3(mat, &node->mat, &res);
        Model *model = node->model;
        if (model && node->alpha() >= 5 && (model->loadstate & 4) && !(node->flg & 0x80000))
        {
            if (g_State.view.bWireframe && !inEditMode())
                xyprintf(10, row++, "%s %d", model->bone_name_offset, node->alpha());
            segs_modelDraw(model, &res, node->trick_node, node->alpha(), node->rgb_entries, nullptr, base_sky_material);
        }
        if (node->children_list)
            segs_gfxTreeDrawNodeSky(node->children_list, &res);
    }
}
void patch_render_node()
{
    BREAK_FUNC(gfxTreeDrawNodeSky);
}
