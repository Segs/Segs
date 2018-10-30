#pragma once
struct GfxTree_Node;
#include <GL/glew.h>
#include <stdint.h>
void segs_modelDrawNode(GfxTree_Node *node);
void segs_modelDrawAlphaSortHackNode(GfxTree_Node *node);
void segs_UVPointer(uint32_t tex_unit, GLint size, GLenum type, GLsizei stride, GLvoid *pointer);

void patch_render_node();
