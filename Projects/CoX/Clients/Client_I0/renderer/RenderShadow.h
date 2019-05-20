#pragma once
#include "utils/helpers.h"
#include <vector>
struct Matrix4x3;
struct SplatSib;

struct ShadowNode
{
    struct Model *model;
    Matrix4x3 mat;
    uint8_t alpha;
    uint8_t shadowMask;
};
static_assert(sizeof(ShadowNode) == 0x38);
extern std::vector<ShadowNode> shadowNodes;
extern int splatShadowsDrawn;
void segs_modelDrawShadowObject(Matrix4x3 *viewSpaceTransform, SplatSib *splat,const struct MaterialDefinition &base);
void  segs_rendertree_drawShadows();
void segs_shadowFinishScene();
void segs_shadowStartScene();
extern void patch_shadow_renderer();
