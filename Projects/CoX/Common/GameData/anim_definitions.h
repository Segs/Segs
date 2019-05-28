/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QString>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include "Common/Runtime/HandleBasedStorage.h"

struct BoneAnimTrack
{
    std::vector<glm::quat> rot_keys;
    std::vector<glm::vec3> pos_keys;
    uint16_t               rotation_ticks;
    uint16_t               position_ticks;
    uint8_t                tgtBoneOrTexId;
};

struct TextureAnim_Data
{
    BoneAnimTrack *animtrack1;
    BoneAnimTrack *animtrack2;
    QByteArray scrollType;
    float speed;
    float stScale;
    int flags;
};
