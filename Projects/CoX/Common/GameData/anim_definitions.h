/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QString>

struct BoneAnimTrack;

struct TextureAnim_Data
{
    BoneAnimTrack *animtrack1;
    BoneAnimTrack *animtrack2;
    QByteArray scrollType;
    float speed;
    float stScale;
    int flags;
};
