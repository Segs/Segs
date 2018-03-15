#pragma once
struct Matrix4x3;
struct SplatSib;

void segs_modelDrawShadowObject(Matrix4x3 *viewSpaceTransform, SplatSib *splat);
extern void patch_shadow_renderer();
