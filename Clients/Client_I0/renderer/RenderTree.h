#pragma once
#include "utils/helpers.h"
#include "RenderBonedModel.h"
#include <vector>

struct GfxTree_Node;
struct Model;
struct SortThing
{
    char blendMode;
    char modelSource;
    union
    {
        GfxTree_Node* gfxnode;
        int vsIdx;
    };
    union
    {
        float distsq;
        Model* model;
    };
};
static_assert(sizeof(SortThing) == 0xC);
struct ViewSortNode
{
    Matrix4x3 mat;
    Vector3 mid;
    Model *model;
    uint8_t alpha;
    char has_tint;
    char has_texids;
    float distsq;
    GLuint rgbs;
    struct EntLight *light;
    RGBA tint_colors[2];
    struct TextureBind *tex_binds[2];
};
static_assert(sizeof(ViewSortNode) == 0x60);

extern std::vector<SortThing> ModelArrayTypeSort;
extern std::vector<SortThing> ModelArrayDistSort;
extern std::vector<ViewSortNode> vsArray;
extern TextureBind *g_global_texbinds[3];
extern int nodeTotal;
extern int NodesDrawn;
extern int drawGfxNodeCalls;
extern int sortedByDist;
extern int sortedByType;
extern int viewspaceMatCount;

extern void segs_gfxTreeDrawNode(GfxTree_Node *basenode, const Matrix4x3 &parent_mat);
extern void segs_rendertree_modelDrawWorldmodel(ViewSortNode *vs);
extern void segs_modelDrawGfxNode(struct GfxTree_Node *node);
extern void patch_rendertree();
void segs_addViewSortNode(GfxTree_Node *node, Model *model, Matrix4x3 *mat, Vector3 *mid, int alpha, GLuint rgbs, RGBA *tint_colors, TextureBind **custom_texbinds, EntLight *light);
void segs_modelAddShadow(Matrix4x3 *mat, uint8_t alpha, Model *model, uint8_t mask);
