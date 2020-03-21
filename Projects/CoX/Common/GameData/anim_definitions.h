/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
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
    int8_t                 tgtBoneOrTexId;
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
namespace SEGS
{
struct BoneLink
{
    int32_t  child_id;
    int32_t  next_bone_idx;
    uint32_t id;
};
class AnimationStorage;
struct AnimTrack
{
    using StorageClass = AnimationStorage; //tells the handle template to look up
    std::vector<BoneAnimTrack> m_bone_tracks;
    std::vector<BoneLink>      m_skeleton_hierarchy;
    QByteArray                 m_name;
    QByteArray                 m_parent_track_name;
    SingularStoreHandleT<20,12,AnimTrack> m_backup_anim_track;
    int                        m_size;
    float                      m_max_hip_displacement;
    float                      m_length;
    int                        m_last_change_date;
};
using HAnimationTrack = SingularStoreHandleT<20,12,AnimTrack>;
}
BoneAnimTrack *getTrackForBone(SEGS::HAnimationTrack trk, int bone);
