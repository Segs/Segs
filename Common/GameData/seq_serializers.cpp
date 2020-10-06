/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
 /*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "seq_serializers.h"

#include "seq_definitions.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <cereal/types/unordered_map.hpp>

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QDir>

namespace
{

    bool loadFrom(BinStore *s, SeqCycleMoveData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SeqNextMoveData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SeqMoveDataTypeAnim *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->read(target->firstFrame);
        ok &= s->read(target->lastFrame);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SeqPlayFxData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->read(target->delay);
        ok &= s->read(target->flags);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SeqMoveTypeData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->read(target->Scale);
        ok &= s->read(target->MoveRate);
        ok &= s->read(target->SmoothSprint);
        ok &= s->read(target->PitchAngle);
        ok &= s->read(target->PitchRate);
        ok &= s->read(target->PitchStart);
        ok &= s->read(target->PitchEnd);
        ok &= s->prepare_nested(); // will update the file size left
        assert(ok);

        if(s->end_encountered())
            return ok;

        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("Anim"==_name)
            {
                SeqMoveDataTypeAnim nt;
                ok &= loadFrom(s,&nt);
                target->m_Anim.push_back(nt);
            }
            else if("PlayFx"==_name)
            {
                SeqPlayFxData nt;
                ok &= loadFrom(s,&nt);
                target->m_PlayFx.push_back(nt);
            }
            else
                assert(!"unknown field referenced.");
             s->nest_out();
        }
        return ok;
    }

    bool loadFrom(BinStore *s, SeqMoveData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->read(target->MoveRate);
        ok &= s->read(target->Interpolate);
        ok &= s->read(target->Priority);
        ok &= s->read(target->Scale);
        ok &= s->read(target->Member);
        ok &= s->read(target->Interrupts);
        ok &= s->read(target->SticksOnChild);
        ok &= s->read(target->SetsOnChild);
        ok &= s->read(target->Sets);
        ok &= s->read(target->Requires);
        ok &= s->read(target->Flags);
        ok &= s->prepare_nested(); // will update the file size left

        assert(ok);
        if(s->end_encountered())
            return ok;

        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("NextMove"==_name)
            {
                SeqNextMoveData nt;
                ok &= loadFrom(s,&nt);
                target->m_NextMove.push_back(nt);
            }
            else if("CycleMove"==_name)
            {
                SeqCycleMoveData nt;
                ok &= loadFrom(s,&nt);
                target->m_CycleMove.push_back(nt);
            }
            else if("Type"==_name)
            {
                SeqMoveTypeData nt;
                ok &= loadFrom(s,&nt);
                target->m_Type.push_back(nt);
            }
            else
                assert(!"unknown field referenced.");

            assert(ok);
            s->nest_out();
        }
        return ok;
    }

    bool loadFrom(BinStore *s, SeqGroupNameData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SeqTypeDefData *target)
    {
        bool ok = true;
        s->prepare();
        ok &= s->read(target->name);
        ok &= s->read(target->pBaseSkeleton);
        ok &= s->read(target->pParentType);
        ok &= s->prepare_nested(); // will update the file size left

        assert(s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, SequencerData *target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->name);
        ok &= s->prepare_nested(); // will update the file size left

        if(s->end_encountered())
            return ok;

        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("TypeDef"==_name)
            {
                SeqTypeDefData nt;
                ok &= loadFrom(s,&nt);
                assert(ok);
                target->m_TypeDef.push_back(nt);
            }
            else if("Group"==_name)
            {
                SeqGroupNameData nt;
                ok &= loadFrom(s,&nt);
                assert(ok);
                target->m_Group.push_back(nt);
            }
            else if("Move"==_name)
            {
                SeqMoveData nt;
                ok &= loadFrom(s,&nt);
                assert(ok);
                target->m_Move.push_back(nt);
            }
            else
                assert(!"unknown field referenced.");

            s->nest_out();
        }
        assert(ok);
        return ok;
    }
} // namespace

bool loadFrom(BinStore *s, SequencerList &target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left

    if(s->end_encountered())
        return ok;

    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Sequencer"==_name)
        {
            SequencerData nt;
            ok &= loadFrom(s,&nt);
            cleanSeqFileName(nt.name);       // this was done after reading, no reason to not do this now.
            target.sq_list.push_back(nt);
            target.m_Sequencers[nt.name.toLower()] = target.sq_list.size()-1;
            s->nest_out();
        }
        else
            assert(!"unknown field referenced.");
    }

    assert(ok);
    return ok;
}
template<class Archive>
void serialize(Archive & archive, SeqPlayFxData & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("delay",m.delay));
    archive(cereal::make_nvp("flags",m.flags));
}
template<class Archive>
void serialize(Archive & archive, SeqMoveDataTypeAnim & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("firstFrame",m.firstFrame));
    archive(cereal::make_nvp("lastFrame",m.lastFrame));
}
template<class Archive>
void serialize(Archive & archive, SeqMoveTypeData & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("Anim",m.m_Anim));
    archive(cereal::make_nvp("PlayFx",m.m_PlayFx));
    archive(cereal::make_nvp("Scale",m.Scale));
    archive(cereal::make_nvp("MoveRate",m.MoveRate));
    archive(cereal::make_nvp("PitchAngle",m.PitchAngle));
    archive(cereal::make_nvp("PitchRate",m.PitchRate));
    archive(cereal::make_nvp("PitchStart",m.PitchStart));
    archive(cereal::make_nvp("PitchEnd",m.PitchEnd));
    archive(cereal::make_nvp("SmoothSprint",m.SmoothSprint));
}
template<class Archive>
void serialize(Archive & archive, SeqNextMoveData & m)
{
    archive(cereal::make_nvp("name",m.name));
}
template<class Archive>
void serialize(Archive & archive, SeqCycleMoveData & m)
{
    archive(cereal::make_nvp("name",m.name));
}
template<class Archive>
void serialize(Archive & archive, SeqMoveData & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("Scale",m.Scale));
    archive(cereal::make_nvp("MoveRate",m.MoveRate));
    archive(cereal::make_nvp("Interpolate",m.Interpolate));
    archive(cereal::make_nvp("Priority",m.Priority));
    archive(cereal::make_nvp("Flags",m.Flags));
    archive(cereal::make_nvp("NextMove",m.m_NextMove));
    archive(cereal::make_nvp("CycleMove",m.m_CycleMove));
    archive(cereal::make_nvp("Type",m.m_Type));
    archive(cereal::make_nvp("SticksOnChild",m.SticksOnChild));
    archive(cereal::make_nvp("SetsOnChild",m.SetsOnChild));
    archive(cereal::make_nvp("Sets",m.Sets));
    archive(cereal::make_nvp("Requires",m.Requires));
    archive(cereal::make_nvp("Member",m.Member));
    archive(cereal::make_nvp("Interrupts",m.Interrupts));
}
template<class Archive>
void serialize(Archive & archive, SeqGroupNameData & m)
{
    archive(cereal::make_nvp("name",m.name));
}
template<class Archive>
void serialize(Archive & archive, SeqTypeDefData & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("BaseSkeleton",m.pBaseSkeleton));
    archive(cereal::make_nvp("ParentType",m.pParentType));
}
template<class Archive>
void serialize(Archive & archive, SequencerData & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("TypeDef",m.m_TypeDef));
    archive(cereal::make_nvp("Group",m.m_Group));
    archive(cereal::make_nvp("Move",m.m_Move));
}

void saveTo(const SequencerList & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target.sq_list,"SequencerList",baseName,text_format);
}

template<class Archive>
static void serialize(Archive & archive, EntitySequencerData & m)
{
    serialize_as_optional(archive,"Name",m.m_name);
    archive(cereal::make_nvp("Sequencer",m.m_sequencer_name));
    serialize_as_optional(archive,"SequencerType",m.m_seq_type);
    serialize_as_optional(archive,"Graphics",m.m_graphics);
    serialize_as_optional(archive,"LOD1_Gfx",m.m_lod_names[0]);
    serialize_as_optional(archive,"LOD2_Gfx",m.m_lod_names[1]);
    serialize_as_optional(archive,"LOD3_Gfx",m.m_lod_names[2]);
    serialize_as_optional(archive,"LOD0_Dist",m.m_lod_dists[0]);
    serialize_as_optional(archive,"LOD1_Dist",m.m_lod_dists[1]);
    serialize_as_optional(archive,"LOD2_Dist",m.m_lod_dists[2]);
    serialize_as_optional(archive,"LOD3_Dist",m.m_lod_dists[3]);
    serialize_as_optional(archive,"VisSphereRadius",m.m_visibility_sphere_radius);
    serialize_as_optional(archive,"MaxAlpha",m.m_max_alpha);
    serialize_as_optional(archive,"ReverseFadeOutDist",m.m_reverse_fade_out_distance);
    serialize_as_optional(archive,"FadeOutStart",m.m_fade_out_start);
    serialize_as_optional(archive,"FadeOutFinish",m.m_fade_out_finish);
    serialize_as_optional(archive,"Shadow",m.m_shadow);
    serialize_as_optional(archive,"UseShadow",m.m_use_shadow);
    serialize_as_optional(archive,"TicksToLingerAfterDeath",m.m_ticks_to_linger_after_death);
    serialize_as_optional(archive,"TicksToFadeAwayAfterDeath",m.m_ticks_to_fade_away_after_death);
    serialize_as_optional(archive,"ShadowType",m.m_shadow_type);
    serialize_as_optional(archive,"ShadowTexture",m.m_shadow_texture);
    serialize_as_optional(archive,"ShadowQuality",m.m_shadow_quality);
    serialize_as_optional(archive,"ShadowSize",m.m_shadow_size);
    serialize_as_optional(archive,"ShadowOffset",m.m_shadow_offset);
    serialize_as_optional(archive,"Flags",m.m_flags);
    serialize_as_optional(archive,"LightAsDoorOutside",m.m_light_as_door_outside);
    archive(cereal::make_nvp("GeomScale",m.m_geometry_scale));
    serialize_as_optional(archive,"GeomScaleMax",m.m_geometry_scale_max);
    serialize_as_optional(archive,"AnimScale",m.m_anim_scale);
    serialize_as_optional(archive,"CollisionSize",m.m_collision_size);
    serialize_as_optional(archive,"CollisionOffset",m.m_collision_offset);
    serialize_as_optional(archive,"HasRandomName",m.m_has_random_name);
    serialize_as_optional(archive,"Fx",m.m_effect_names[0]);
    serialize_as_optional(archive,"Fx2",m.m_effect_names[1]);
    serialize_as_optional(archive,"Fx3",m.m_effect_names[2]);
    serialize_as_optional(archive,"Fx4",m.m_effect_names[3]);
    serialize_as_optional(archive,"Fx5",m.m_effect_names[4]);
    serialize_as_optional(archive,"WorldGroup",m.m_world_group);
    archive(cereal::make_nvp("MinimumAmbient",m.m_minimum_ambient));
    serialize_as_optional(archive,"BoneScaleFat",m.m_bone_scale_fat);
    serialize_as_optional(archive,"BoneScaleSkinny",m.m_bone_scale_skinny);
    serialize_as_optional(archive,"RandomBoneScale",m.m_random_bone_scale);
    serialize_as_optional(archive,"NotSelectable",m.m_not_selectable);
    serialize_as_optional(archive,"NoCollision",m.m_no_collision);
    serialize_as_optional(archive,"CollisionType",m.m_collision_type);
    serialize_as_optional(archive,"Bounciness",m.m_bounciness);
    serialize_as_optional(archive,"Placement",m.m_placement);
    serialize_as_optional(archive,"Selection",m.m_selection);
    serialize_as_optional(archive,"ConstantState",m.m_constant_state);
    serialize_as_optional(archive,"CapeFile",m.m_cape_file);
    serialize_as_optional(archive,"CapeName",m.m_cape_name);
    serialize_as_optional(archive,"CapeHarnessFile",m.m_cape_harness_file);
    serialize_as_optional(archive,"CapeHarnessName",m.m_cape_harness_name);
    serialize_as_optional(archive,"CapeTex1",m.m_cape_textures[0]);
    serialize_as_optional(archive,"CapeTex2",m.m_cape_textures[1]);
    serialize_as_optional(archive,"CapeColor1",m.m_cape_colors[0]);
    serialize_as_optional(archive,"CapeColor2",m.m_cape_colors[1]);
    serialize_as_optional(archive,"CapeInnerTex1",m.m_cape_inner_textures[0]);
    serialize_as_optional(archive,"CapeInnerTex2",m.m_cape_inner_textures[1]);
    serialize_as_optional(archive,"CapeInnerColor1",m.m_inner_cape_colors[0]);
    serialize_as_optional(archive,"CapeInnerColor2",m.m_inner_cape_colors[1]);
    serialize_as_optional(archive,"ReticleHeightBias",m.m_reticle_mod.y);
    serialize_as_optional(archive,"ReticleWidthBias",m.m_reticle_mod.x);
}

bool loadFrom(const QString &path, SequencerTypeMap &target)
{
    QFile ifl(path);
    if(path.endsWith("json") || path.endsWith("crl_json"))
    {
        if(!ifl.open(QFile::ReadOnly|QFile::Text))
        {
            qWarning() << "Failed to open" << path;
            return false;
        }
        std::istringstream istr(ifl.readAll().toStdString());

        cereal::JSONInputArchive arc(istr);
        arc(cereal::make_nvp("SequencerTypes", target));
    }
//    else if(path.endsWith("bin")), no support for optional fields in binary archive
//    {
//        if(!ifl.open(QFile::ReadOnly))
//        {
//            qWarning() << "Failed to open" << path;
//            return false;
//        }
//        std::istringstream istr(ifl.readAll().toStdString());

//        cereal::BinaryInputArchive arc(istr);
//        arc(cereal::make_nvp("EntitySequencer", target));
//    }
    else
    {
        qWarning() << "Invalid serialized data extension in" <<path;
        return false;
    }

    return true;
}
void saveTo(const SequencerTypeMap &target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"SequencerTypes",baseName,text_format);
}

//! @}
