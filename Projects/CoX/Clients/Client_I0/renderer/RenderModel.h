#pragma once

#include <GL/glew.h>
struct GfxTree_Node;
struct Model;
struct Vector3;
struct EntLight;
struct MaterialDefinition;

extern int nonboned;
extern int boned;
extern int drawOrder;
extern int g_nextdraw;
void segs_modelDrawNode(GfxTree_Node *node,const MaterialDefinition &matdef);
void segs_modelDrawAlphaSortHackNode(GfxTree_Node *node);
void segs_gfxTreeDrawNodeSky(GfxTree_Node *skynode, struct Matrix4x3 *mat);
void segs_modelDrawAlphaSortHack(Model *model, Matrix4x3 *mat, int alpha, Vector3 *mid, GLuint rgbs, EntLight *light);

void patch_render_node();
