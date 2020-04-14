#pragma once
struct Vector3;
void drawLine3D_2Color(const Vector3 *p1, unsigned int argb_pt1, const Vector3 *p2, unsigned int argb_pt2);
void drawLine3D_z_2Color(const Vector3 *p1, unsigned int argb_pt1, const Vector3 *p2, unsigned int argb_pt2);
extern void patch_ent_debug();
