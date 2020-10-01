/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
/**
    @file Sequencer.h
    @brief provides animation control ( triggers/transitions ) and sequencing (ordering/merging) functionality
*/

#include "GameData/anim_definitions.h"
#include "GameData/seq_definitions.h"
#include "Runtime/Animation.h"
#include "Runtime/HandleBasedStorage.h"
#include "glm/mat4x3.hpp"
#include "glm/vec4.hpp"
#include <array>
#include <unordered_map>
#include <unordered_set>


#include "GameData/anim_definitions.h"
#include "GameData/anim_definitions.h"
class QByteArray;
struct SequencerData;

namespace SEGS
{
struct GeoSet;
struct SequencerInstanceStorage;
struct SceneTreeNode;
struct TextureWrapper;

enum class CollisionType
{
    None         = 0,
    Repulsion    = 1,
    Rubberball   = 2,
    BounceUp     = 3,
    BounceFacing = 4,
    SteadyUp     = 5,
    SteadyFacing = 6,
    Door         = 7,
};
enum class SelectionMode
{
    Bones      = 1,
    Collision  = 2,
    Worldgroup = 3,
};
struct ModelPartColor
{
    glm::vec4 color1;
    glm::vec4 color2;
    bool      operator==(const ModelPartColor &o) const { return color1 == o.color1 && color2 == o.color2; }
};
struct SeqMove
{
    SeqMoveData *    move           = nullptr;
    SeqMoveData *    move_to_send   = nullptr;
    SeqMoveData *    prev_move      = nullptr;
    SeqMoveData *    move_lastframe = nullptr;
    SeqMoveTypeData *type           = nullptr;
    float            frame          = 0; // frame time
    float            prev_frame     = 0;
};

struct RenderingData
{
    glm::vec3   m_bone_translations[70];
    glm::mat4x3 m_bone_matrices[70];
    int         m_alpha = 0;
};
struct SeqAnimation
{
    HAnimationTrack m_base_track;
};
struct SequencerInstance
{
    using StorageClass = SEGS::SequencerInstanceStorage;
    SequencerInstance(SceneTreeNode *node = nullptr);

    EntitySequencerData         m_seq_type_info;
    SequencerData *             m_template;
    uint8_t                     m_next_move_idx = 0;
    struct SceneTreeNode *      m_node          = nullptr; // root of the node hierarchy this sequencer is controlling.
    std::vector<ModelPartColor> m_part_colors;
    glm::vec3                   m_current_geom_scale;
    float                       m_current_animation_scale;
    SeqBitSet                   m_current_state_bits;

    float                           m_bone_scale_ratio;
    GeoSet *                        m_fat_bodytype_animation    = nullptr;
    GeoSet *                        m_skinny_bodytype_animation = nullptr;
    SeqMove                         m_anim;
    int                             m_rand_seed;
    glm::vec3                       m_bone_scales[70];
    glm::vec4                       m_last_rotations[70];
    glm::vec3                       m_last_translations[70];
    bool                            m_bone_has_color[70];
    ModelPartColor                  m_bone_colors[70];
    HandleT<20, 12, TextureWrapper> m_bone_textures_1[70];
    HandleT<20, 12, TextureWrapper> m_bone_textures_2[70];
    std::array<uint8_t, 70>         m_interpolation_state;
    QByteArray                      m_bone_geometry_names[70];
    RenderingData                   m_rendering_data;
    SeqAnimation                    m_animation;
    int                             m_curr_interpolated_frame = 0;
    int                             m_lod_level               = 0;
    bool                            m_updated_appearance      = false; // set to tru if the appearance has changed
};
struct SequencerInstanceStorage : public HandleBasedStorage<SequencerInstance>
{
    using Type = SequencerInstance;
    std::unordered_set<SequencerData *> m_active_instances;
    static SequencerInstanceStorage &   instance()
    {
        static SequencerInstanceStorage s_instance;
        return s_instance;
    }
};
using HSequencerInstance = SingularStoreHandleT<20, 12, SequencerInstance>;

SequencerData * getInitializedSequencerData(const QByteArray &name);
void            seqResetSeqType(HSequencerInstance seq_handle, FSWrapper &fs, const char *entType_filename, int seed);
SeqTypeDefData *getSeqTypedefByName(SequencerData *data, const QByteArray &name);
bool            changeSequencerScale(HSequencerInstance seq, const glm::vec3 &scale);
void            changeBoneScale(HSequencerInstance seq, float newbonescale);
} // namespace SEGS
