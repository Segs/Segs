#include "Animation.h"
#include "Model.h"
#include "Prefab.h"
#include "RuntimeData.h"

#include "AnimationEngine.h"
#include "GameData/anim_definitions.h"

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QSet>
#include <cassert>

using namespace SEGS;

namespace
{
// animated geometries
QHash<QString, GeoSet *> g_geoset_dictionary;
QSet<QString>            g_missing_geos;
// file structure layouts
#pragma pack(push, 1)
struct PackedAnimPos
{
    int16_t x, y, z;
};
struct PackedAnimRot
{
    char packed[5];
};
static_assert(sizeof(PackedAnimRot) == 5, "PackedAnimRot must be 5 bytes");
struct PackedAnimRot2
{
    int16_t x, y, z, w;
};
struct AnimTrack_Entry32
{
    int      rot_idx;
    int      pos_idx;
    uint16_t rot_fullkeycount;
    uint16_t pos_fullkeycount;
    uint16_t rotation_keyframes;
    uint16_t position_keyframes;
    char     tgtBoneOrTexId;
    uint8_t  encoding_type;
    uint16_t u1;
};

#pragma pack(pop)
struct AnimTrack32
{
    int      size;
    char     name1[256];
    char     parent_track_name[256];
    int      max_hip_displacement;
    float    length;
    int      entries; // offset to track entries
    uint32_t num_entries;
    int      u_1;
    int      u_2;
    int      m_skeleton_hierarchy;
    int      backupAnimTrack;
    int      u_3;
    int      u_4;
    int      last_change_date;
};
void initLookUpTable(float *tab)
{
    float div_factor = 1.0f / std::sqrt(2.0f);
    // conversion table 0-4096 to -1.0, 1.0
    for (int i = 0; i < 4096; ++i)
    {
        float centered = (float(2 * i) / 4096.0f - 1); // float(2*i) / 4096.0f => [0,2.0]
        tab[i]         = div_factor * centered;
    }
}
void unPack5ByteQuat(const PackedAnimRot &cquat, glm::ivec3 *tgt, int8_t *missing)
{
    char    part1;
    int32_t part2;

    assert(cquat.packed[0] < 64);
    memcpy(&part2, cquat.packed + 1, sizeof(int32_t));
    part1    = cquat.packed[0];
    *missing = part1 >> 4;
    tgt->z   = part2 & 0xFFF;
    tgt->y   = (part2 >> 12) & 0xFFF;
    tgt->x   = (part2 >> 24) | ((part1 & 0xF) << 8);
    assert(*missing <= 3);
}
void animExpand5ByteQuat(const PackedAnimRot &cquat, glm::quat &quat)
{
    static float s_table12bit[4096];
    static bool  s_table_ready = false;

    int        cmp_idx = 0;
    int8_t     missing;
    glm::ivec3 qidxs;

    if (!s_table_ready)
    {
        initLookUpTable(s_table12bit);
        s_table_ready = true;
    }
    unPack5ByteQuat(cquat, &qidxs, &missing);

    if (missing != 0)
        quat.x = s_table12bit[qidxs[cmp_idx++]];
    if (missing != 1)
        quat.y = s_table12bit[qidxs[cmp_idx++]];
    if (missing != 2)
        quat.z = s_table12bit[qidxs[cmp_idx++]];
    if (missing != 3)
        quat.w = s_table12bit[qidxs[cmp_idx++]];

    quat[missing] = 0.0f;
    glm::bvec4 allfalse(false, false, false, false);
    assert(allfalse == glm::isinf(quat) && allfalse == glm::isnan(quat)); // make sure the reconstructed value is valid
    float sqr;
    // restore the missing component
    // since the idea was that the largest component will be restored, in each case that is verified with an assert
    switch (missing)
    {
    case 3:
    {
        sqr = 1.0f - quat.z * quat.z - quat.y * quat.y - quat.x * quat.x;
        assert(sqr > -0.1f);
        quat.w = std::sqrt(std::max(0.0f, sqr));
        assert(quat.w >= quat.x - 0.001f && quat.w >= quat.y - 0.001f && quat.w >= quat.z - 0.001f);
        break;
    }
    case 2:
    {
        // this fixup for z component of the quaternion seems strange, but was retained to keep the parity with
        // original decoding procedure
        sqr = 1.0f - quat.w * quat.w - quat.y * quat.y - quat.x * quat.x + 0.001f;
        assert(sqr > -0.001f);
        quat.z = std::sqrt(std::max(0.0f, sqr));
        assert(quat.z >= quat.x - 0.001f && quat.z >= quat.y - 0.001f && quat.z >= quat.w - 0.001f);
        break;
    }
    case 1:
    {
        sqr = 1.0f - quat.w * quat.w - quat.z * quat.z - quat.x * quat.x;
        assert(sqr > -0.001f); //, "\\src\\Common\\seq\\animtrackanimate.c", 0x153);
        quat.y = std::sqrt(std::max(0.0f, sqr));
        assert(quat.y >= quat.x - 0.001f && quat.y >= quat.z - 0.001f && quat.y >= quat.w - 0.001f);
        break;
    }
    case 0:
    {
        sqr = 1.0f - quat.w * quat.w - quat.z * quat.z - quat.y * quat.y;
        assert(sqr > -0.001f);
        quat.x = std::sqrt(std::max(0.0f, sqr));
        assert(quat.x >= quat.y - 0.001f && quat.x >= quat.z - 0.001f && quat.x >= quat.w - 0.001f);
        break;
    }
    default: break;
    }
    assert(allfalse == glm::isinf(quat) && allfalse == glm::isnan(quat)); // make sure the reconstructed value is valid
}

bool validBoneIdx(int idx)
{
    return idx >= 0 && idx < 70;
}
void convertBoneHierarchy(const BoneLink *src, int start_bone_id, std::vector<BoneLink> &tgt)
{
    for (int idx = start_bone_id; idx != -1; idx = src[idx].next_bone_idx)
    {
        assert(validBoneIdx(idx));
        if (idx >= int(tgt.size()))
            tgt.resize(size_t(idx + 1));
        tgt[idx] = src[idx];
        if (src[idx].child_id != -1)
            convertBoneHierarchy(src, src[idx].child_id, tgt);
    }
}
static void convertToAnimTrack(char *data_as_raw, AnimTrack &tgt)
{
    // verify source structure sizes match the original layout
    static_assert(sizeof(AnimTrack32) == 0x230, "Bad size for animtrack32");
    static_assert(sizeof(AnimTrack_Entry32) == 0x14, "Bad size for AnimTrack_Entry32");
    static_assert(sizeof(BoneLink) == 0xC, "Bad size for BoneLink");

    auto header_data = reinterpret_cast<const AnimTrack32 *>(data_as_raw);

    if (0 != header_data->m_skeleton_hierarchy)
    {
        int  start_id = *(int *)(data_as_raw + header_data->m_skeleton_hierarchy);
        auto skeletonHierarchy =
            reinterpret_cast<const BoneLink *>(4 + header_data->m_skeleton_hierarchy + data_as_raw);
        convertBoneHierarchy(skeletonHierarchy, start_id, tgt.m_skeleton_hierarchy);
    }
    tgt.m_size              = header_data->size;
    tgt.m_name              = header_data->name1;
    tgt.m_parent_track_name = header_data->parent_track_name;

    tgt.m_bone_tracks.resize(header_data->num_entries);
    tgt.m_max_hip_displacement = header_data->max_hip_displacement;
    tgt.m_length               = header_data->length;

    tgt.m_last_change_date = header_data->last_change_date;

    auto _entries = reinterpret_cast<const AnimTrack_Entry32 *>(data_as_raw + header_data->entries);
    for (uint32_t i = 0; i < header_data->num_entries; ++i)
    {
        BoneAnimTrack &          converted(tgt.m_bone_tracks[i]);
        const AnimTrack_Entry32 &track_entry_source(_entries[i]);
        converted.pos_keys.resize(track_entry_source.pos_fullkeycount);
        converted.rot_keys.resize(track_entry_source.rot_fullkeycount);
        converted.rotation_ticks = track_entry_source.rotation_keyframes;
        converted.position_ticks = track_entry_source.position_keyframes;
        converted.tgtBoneOrTexId = track_entry_source.tgtBoneOrTexId;

        uint8_t e_type = track_entry_source.encoding_type;
        if (track_entry_source.rot_idx)
        {
            uint8_t rot_type    = (e_type & 0x7);
            bool    correct_rot = (rot_type == 1) || (rot_type == 2) || (rot_type == 4);
            assert(correct_rot && "Cannot have both kinds of rotation keys");
            if (rot_type & 2)
            {
                PackedAnimRot *src_rots = (PackedAnimRot *)(data_as_raw + track_entry_source.rot_idx);
                for (uint16_t i = 0; i < track_entry_source.rot_fullkeycount; ++i)
                {
                    animExpand5ByteQuat(src_rots[i], converted.rot_keys[i]);
                    assert(converted.rot_keys[i] != glm::quat(0, 0, 0, 0));
                }
            }
            else if (rot_type & 1)
            {
                glm::quat *src_rots = (glm::quat *)(data_as_raw + track_entry_source.rot_idx);
                for (uint16_t i = 0; i < track_entry_source.rot_fullkeycount; ++i)
                {
                    converted.rot_keys[i] = src_rots[i];
                    assert(converted.rot_keys[i] != glm::quat(0, 0, 0, 0));
                }
            }
            else if (rot_type & 4)
            {
                PackedAnimRot2 *src_rots = (PackedAnimRot2 *)(data_as_raw + track_entry_source.rot_idx);
                for (uint16_t i = 0; i < track_entry_source.rot_fullkeycount; ++i)
                {
                    converted.rot_keys[i] = {src_rots[i].x / 10000.0f, src_rots[i].y / 10000.0f,
                                             src_rots[i].z / 10000.0f, src_rots[i].w / 10000.0f};
                    assert(converted.rot_keys[i] != glm::quat(0, 0, 0, 0));
                }
            }
        }
        if (track_entry_source.pos_idx)
        {
            uint8_t pos_type    = (e_type & 0x18);
            bool    correct_pos = (pos_type == 8) || (pos_type == 0x10);
            assert(correct_pos && "Cannot have multiple types of position keys");
            if (pos_type & 8)
            {
                glm::vec3 *src_pos = (glm::vec3 *)(data_as_raw + track_entry_source.pos_idx);
                for (int i = 0; i < track_entry_source.pos_fullkeycount; ++i)
                {
                    converted.pos_keys[i] = src_pos[i];
                }
            }
            else if (pos_type & 0x10)
            {
                PackedAnimPos *src_pos = (PackedAnimPos *)(data_as_raw + track_entry_source.pos_idx);
                for (uint16_t i = 0; i < track_entry_source.pos_fullkeycount; ++i)
                {
                    glm::vec3 p(src_pos[i].x / 32000.0f, src_pos[i].y / 32000.0f, src_pos[i].z / 32000.0f);
                    converted.pos_keys[i] = p;
                }
            }
            else
                assert(!"Unknown Pos key type");
        }
    }
}

HAnimationTrack animReadTrackFile(QFile *fp)
{
    QByteArray      binary_data = fp->readAll();
    HAnimationTrack handle      = AnimationStorage::instance().create();
    convertToAnimTrack(binary_data.data(), handle.get());
    return handle;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end of anonymous namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAnimationTrack getOrLoadAnimationTrack(const QByteArray &name)
{
    RuntimeData &rd(getRuntimeData());
    QString      base_path = rd.m_prefab_mapping->m_base_path;
    char         key[256]  = {0};

    if (name.isEmpty())
    {
        qCritical() << "getAnimTrack needs a file name.";
        return {};
    }

    assert(name.size() < 256);
    strcpy(key, qPrintable(name.toUpper()));
    QByteArray      latin_name     = name;
    QString         full_anim_path = ("player_library/animations/"+name+".anim").toLower();
    HAnimationTrack animTrack      = AnimationEngine::get().m_loaded_tracks.value(latin_name.toUpper(), {});
    if (animTrack)
        return animTrack;

    QFile anim_file(base_path + "/" + full_anim_path);
    if (!anim_file.open(QFile::ReadOnly))
    {
        qCritical() << "failed to open animation file" << full_anim_path;
        return {};
    }
    animTrack = animReadTrackFile(&anim_file);

    AnimationEngine::get().m_loaded_tracks[latin_name.toUpper()] = animTrack;
    SEGS::AnimTrack &atrack(animTrack.get());
    HAnimationTrack  backup = getOrLoadAnimationTrack(atrack.m_parent_track_name);
    // make sure at least the backup animation is sane.
    assert(backup && !backup->m_skeleton_hierarchy.empty());
    atrack.m_backup_anim_track = backup;
    // TODO: consider shared/cow memory for skeleton hierarchies ?
    atrack.m_skeleton_hierarchy   = HAnimationTrack(atrack.m_backup_anim_track)->m_skeleton_hierarchy;
    atrack.m_max_hip_displacement = 4.0f;
    return animTrack;
}
static GeoSet *getAnimatedGeoSet(FSWrapper &fs,const QByteArray &name, QIODevice *&fp)
{
    RuntimeData &rd(getRuntimeData());
    QByteArray      base_path = rd.m_prefab_mapping->m_base_path;
    fp = fs.open(base_path + "/" + name,true);
    if (!fp)
    {
        qWarning() << "Failed to open" << name;
        g_missing_geos.insert(name.toLower());
        return nullptr;
    }
    GeoSet *geoset = new GeoSet;
    geoset->name = name;
    geosetLoadHeader(fp, geoset);
    fp->seek(0);
    g_geoset_dictionary[geoset->name] = geoset;
    return geoset;
}
GeoSet *animLoad(FSWrapper &fs, const QByteArray &filename, bool background_load, bool header_only)
{
    GeoSet *geoset;
    QByteArray animname(filename);

    if (animname.toLower().endsWith(".anm"))
        animname.replace(animname.lastIndexOf("."), 4, ".geo");

    if (animname.isEmpty())
        qCritical("Cannot load animated geometry without a name\n");
    if (g_missing_geos.contains(animname.toLower()))
        return nullptr;

    geoset = g_geoset_dictionary.value(animname, nullptr);
    if (geoset)
    {
        if (geoset->data_loaded || header_only)
            return geoset;

        // TODO: if given geo set is being loaded asynchronously, wait for it.
    }
    QIODevice *file;
    if (!geoset)
        geoset = getAnimatedGeoSet(fs,animname, file);

    if (header_only || !geoset)
        return geoset;

    if (background_load) // Load in thread
    {
        // TODO: implement backgroud loads ?
        assert(false);
    }
    else // load now
    {
        geosetLoadData(file, geoset);
    }
    return geoset;
}
