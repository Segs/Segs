#pragma once
#include <stdint.h>

enum ClientStates : int
{
    BONE_WEIGHTS         = 0,
    BONE_INDICES         = 1,
    BINORMALS            = 2,
    TANGENTS             = 3,
    TEX0_COORDS          = 4,
    TEX1_COORDS          = 5,
    POSITIONS            = 6,
    NORMALS              = 7,
    PERVERTEXCOLORS      = 8,
    PERVERTEXCOLORS_ONLY = 9,
};
enum class DrawMode : int
{
    SINGLETEX = 1,
    DUALTEX = 2,
    COLORONLY = 3,
    DUALTEX_NORMALS = 4,
    SINGLETEX_NORMALS = 5,
    FILL = 6,
    BUMPMAP_SKINNED = 7,
    HW_SKINNED = 8,
    BUMPMAP_NORMALS = 9,
    BUMPMAP_DUALTEX = 10,
    BUMPMAP_RGBS = 11,
};
enum class eBlendMode : int
{
    MULTIPLY=0,
    MULTIPLY_REG=1,
    COLORBLEND_DUAL=2,
    ADDGLOW=3,
    ALPHADETAIL=4,
    BUMPMAP_MULTIPLY=5,
    BUMPMAP_COLORBLEND_DUAL=6,
    INVALID=~MULTIPLY, // all bits set, should be properly sized to underlying type
};
extern void initializeRenderer();
