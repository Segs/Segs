#pragma once

#include <QString>

struct BoneAnimTrack;

struct TextureAnim_Data
{
    BoneAnimTrack *animtrack1;
    BoneAnimTrack *animtrack2;
    QString scrollType;
    float speed;
    float stScale;
    int flags;
};
