#include "RenderTree.h"

#include "Model.h"
#include "RenderModel.h"
#include "RendererState.h"
#include "RenderBonedModel.h"
#include "RenderShadow.h"
#include "RenderTricks.h"
#include "GameState.h"
#include "graphics/GroupDraw.h"

#include "utils/dll_patcher.h"
#include "utils/helpers.h"
#include "RendererUtils.h"

extern "C" {
    __declspec(dllimport) bool legitBone(uint8_t);
    __declspec(dllimport) bool inEditMode();
    __declspec(dllimport) void rendertree_4DDFE0(GfxTree_Node *node);
    __declspec(dllimport) char *strstri(const char *str1, const char *str2);
    __declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);
    __declspec(dllimport) void handleErrorStr(const char *, ...);
    __declspec(dllimport) void scaleMat3Vec3(Matrix4x3 *, Vector3 *);

    __declspec(dllimport) int see_outside;

}
int viewspaceMatCount;
int sortedByDist;
int sortedByType;
int drawGfxNodeCalls;
int NodesDrawn;
int nodeTotal;
TextureBind *g_global_texbinds[3];
std::vector<ViewSortNode> vsArray;
std::vector<SortThing> ModelArrayTypeSort;
std::vector<SortThing> ModelArrayDistSort;

static Matrix4x3 viewspaceMats[6000];

void segs_modelDrawGfxNode(GfxTree_Node *node)
{
    assert(node && node->model);
    assert(node->model->loadstate == 4); // must be loaded ( == 4)
    ++drawGfxNodeCalls;
    MaterialDefinition node_base_material(g_default_mat);
    node_base_material.render_state = RenderState();
    node_base_material.setDrawMode(DrawMode::SINGLETEX);
    node_base_material.setFragmentMode(eBlendMode::MULTIPLY);

    fprintf(stderr,"Draw node %s\n",node->model->parent_anim->headers->name);
    if ( g_State.view.bShadowVol != 2 )
    {
        if ( node->seqHandle )
        {
            node_base_material.render_state.stencil_mode = RenderState::STENCIL_ALWAYS;
            node_base_material.render_state.stencil_ref = 128;
            node_base_material.render_state.setStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            node_base_material.render_state.stencil_mode = RenderState::STENCIL_NONE;
        }
    }
    if ( node->m_node_splats )
    {
        if ( node->m_node_splats->drawMe )
        {
            node->m_node_splats->drawMe = 0;
            segs_modelDrawShadowObject(node->viewspace, node->m_node_splats,node_base_material);
        }
        return;
    }
    if ( node->assigned_cloth_obj )
    {
        assert(false); // clothing rendering is crazy and/or broken
        //fn_4D6D70(node);
    }
    else
    {
        if (node->model->Model_flg1 & OBJ_TREE)
            segs_modelDrawAlphaSortHackNode(node);
        else
            segs_modelDrawNode(node,node_base_material);
    }
}
static int trackerCMP(const DefTracker **a, const DefTracker **b)
{
    if (*a >= *b)
        return *a != *b;
    return -1;
}
void segs_addViewSortNode(GfxTree_Node *node, Model *model, Matrix4x3 *mat, Vector3 *mid, int alpha, GLuint rgbs, RGBA *tint_colors, TextureBind **custom_texbinds, EntLight *light)
{
    SortThing *entry;
    Model *model_l = model;
    float dist_sq = 0.0;
    assert(model);
    assert(vsArray.size() + size_t(nodeTotal) == (ModelArrayTypeSort.size() + ModelArrayDistSort.size()));
    bool is_dist_sorted = false;
    if (alpha != 255 || model->Model_flg1 & (OBJ_TREE | OBJ_ALPHASORT) || (node != nullptr && node->flg & 0x100000))
    {
        dist_sq = mid->z;
        is_dist_sorted = true;
        if(model->trck_node)
            dist_sq += model->trck_node->SortBias;
        if (!model->trck_node || !(model->trck_node->_TrickFlags & 0x4000000))
            dist_sq -= 2 * model_l->model_VisSphereRadius;
    }
    if (!node)
    {
        ViewSortNode sort_entry;
        sort_entry.rgbs = rgbs;
        sort_entry.mid = *mid;
        sort_entry.distsq = dist_sq;
        sort_entry.light = light;
        sort_entry.model = model_l;
        sort_entry.alpha = alpha;
        sort_entry.has_tint = tint_colors!=nullptr;
        if (tint_colors)
        {
            sort_entry.tint_colors[0] = *tint_colors;
            sort_entry.tint_colors[1] = tint_colors[1];
        }
        sort_entry.mat = *mat;
        sort_entry.has_texids = custom_texbinds != nullptr;
        if (custom_texbinds)
        {
            sort_entry.tex_binds[0] = custom_texbinds[0];
            sort_entry.tex_binds[1] = custom_texbinds[1];
        }
        vsArray.emplace_back(sort_entry);
    }
    assert(model->loadstate == LOADED);
    if (is_dist_sorted == 1)
    {
        ModelArrayDistSort.emplace_back();
        entry = &ModelArrayDistSort.back();
        entry->distsq  = dist_sq;
        sortedByDist++;
    }
    else
    {
        assert(model->loadstate == 4); //LOADED
        ModelArrayTypeSort.emplace_back();
        entry = &ModelArrayTypeSort.back();
        entry->model   = model_l;
        sortedByType++;
    }
    if (node)
    {
        ++nodeTotal;
        assert(node->model);
        entry->modelSource = 1;
        entry->blendMode   = node->blendMode;
        entry->gfxnode     = node;
    }
    else
    {
        entry->modelSource = 2;
        entry->blendMode   = model_l->blendState;
        entry->vsIdx       = vsArray.size() - 1;
    }
    assert(vsArray.size() + size_t(nodeTotal) == (ModelArrayTypeSort.size() + ModelArrayDistSort.size()));
}
void segs_gfxTreeDrawNode(GfxTree_Node *basenode,const Matrix4x3 &parent_mat)
{
    Vector3 src;
    Matrix4x3 dest;
    Matrix4x3 res;
    Matrix4x3 tmp;

    for (GfxTree_Node *node_ = basenode; node_; node_ = node_->next)
    {
        if (g_State.view.iAnimDebugFlags)
            rendertree_4DDFE0(node_);
        if (node_->flg & 0x20000 || (node_->seqHandle && !node_->use_flags))
            continue;
        if (g_State.view.bWireframe && !inEditMode() && node_->model)
        {
            if (strstri(node_->model->bone_name_offset, "sunglow"))
                xyprintf(10, 40, "Drawing %s %d", node_->model->bone_name_offset, node_->alpha());
        }
        if (node_->flg & 0x10000)
        {
            assert(node_->seqGfxData);
            if (!see_outside || node_->seqGfxData->tray)
            {
                if (see_outside || !node_->seqGfxData->tray)
                    continue;
                if (!bsearch(&node_->seqGfxData->tray, visibleTrays.data(), visibleTrays.size(),
                    sizeof(DefTracker *),
                    (int(*)(const void *, const void *))trackerCMP))
                {
                    continue;
                }
            }
        }
        if (legitBone(node_->bone_slot) && node_->seqHandle)
        {
            GlobRenderRel *seqGfxData = node_->seqGfxData;
            assert(seqGfxData);
            assert(legitBone(node_->bone_slot));
            Matrix4x3 *bone_mat = &seqGfxData->animMatrices[uint8_t(node_->bone_slot)];
            dest = Unity_Matrix;
            scaleMat3Vec3(&dest, &node_->boneScale);
            res                 = node_->mat * dest;
            *bone_mat           = parent_mat * res;
            src                 = -seqGfxData->boneTranslations[uint8_t(node_->bone_slot)];
            bone_mat->TranslationPart = *bone_mat * src;
            node_->setAlpha(seqGfxData->alpha);
        }
        if (!node_->children_list && !node_->model)
            continue;
        Model *model = node_->model;
        if (!model ||
            !(model->Model_flg1 & OBJ_TREE) && node_->flg & 0x400000 && model->Model_flg1 & OBJ_DRAW_AS_ENT)
        {
            node_->viewspace = &tmp;
        }
        else
        {
            if (viewspaceMatCount >= 6000)
                handleErrorStr("viewspaceMatCount >= MAX_VIEWSPACEMATS");
            node_->viewspace = &viewspaceMats[viewspaceMatCount++];
        }
        *node_->viewspace = parent_mat * node_->mat; // MatMult4x3(parent_mat, &node_->mat, );

        if (node_->model && !(node_->flg & 0x80000) && node_->alpha() >= 5u)
        {
            if (node_->model->loadstate & 4)
            {
                ++NodesDrawn;
                segs_addViewSortNode(node_, node_->model, nullptr, &node_->viewspace->TranslationPart, node_->alpha(), node_->rgb_entries, nullptr, nullptr, nullptr);
            }
        }
        if (node_->children_list)
          segs_gfxTreeDrawNode(node_->children_list, *node_->viewspace);
    }
}
void segs_rendertree_modelDrawWorldmodel(ViewSortNode *vs)
{
    RGBA tint0_bak = RGBA(0);
    RGBA tint1_bak = RGBA(0);
    MaterialDefinition base_material(g_default_mat);
    base_material.setDrawMode(DrawMode::SINGLETEX);
    base_material.setFragmentMode(eBlendMode::MULTIPLY);

    assert(vs->model->loadstate == LOADED);
    if ( vs->has_texids )
    {
        g_global_texbinds[0] = vs->tex_binds[0];
        g_global_texbinds[1] = vs->tex_binds[1];
    }
    if ( vs->has_tint )
    {
        TrickNode *trick = segs_getOrCreateTrickNode(vs->model);
        tint0_bak = trick->TintColor0;
        tint1_bak = trick->TintColor1;
        trick->TintColor0 = vs->tint_colors[0];
        trick->TintColor1 = vs->tint_colors[1];
    }
    Model *model = vs->model;
    if ( model->Model_flg1 & OBJ_TREE )
        segs_modelDrawAlphaSortHack(model, &vs->mat, vs->alpha, &vs->mid, vs->rgbs, vs->light);
    else
        segs_modelDraw(model, &vs->mat, nullptr, vs->alpha, vs->rgbs, vs->light,base_material);
    if ( vs->has_tint )
    {
        TrickNode *trick = vs->model->trck_node;
        trick->TintColor0 = tint0_bak;
        trick->TintColor1 = tint1_bak;
    }
}
void segs_rendertree_SetColors(GfxTree_Node *node, const uint8_t *clr1, const uint8_t *clr2)
{
    node->flg |= 0x200000;
    node->color1.r = clr1[0];
    node->color1.g = clr1[1];
    node->color1.b = clr1[2];
    node->color2.r = clr2[0];
    node->color2.g = clr2[1];
    node->color2.b = clr2[2];
}
void segs_modelAddShadow(Matrix4x3 *mat, uint8_t alpha, Model *model, uint8_t mask)
{

    ShadowNode snode;
    snode.mat = *mat;
    snode.alpha = alpha;
    snode.model = model;
    snode.shadowMask = mask;
    shadowNodes.emplace_back(snode);
}
void patch_rendertree()
{
    BREAK_FUNC(modelDrawGfxNode);
    BREAK_FUNC(addViewSortNode);
    PATCH_FUNC(rendertree_SetColors);
}

