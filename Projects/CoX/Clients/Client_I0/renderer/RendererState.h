#ifndef RENDERERSTATE_H
#define RENDERERSTATE_H


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

#endif // RENDERERSTATE_H
