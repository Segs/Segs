/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
 /*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "seq_serializers.h"

#include "seq_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"
#include "serialization_types.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QDir>
//#include <optional> // Requires C++17
//template<class Archive,class Type>
//static void serialize_as_optional(Archive & archive, const char *name, Type & m)
//{
//    std::optional<Type> opt_str;
//    if(!(m==Type()))
//        opt_str = m;

//    archive(cereal::make_nvp(name,opt_str));
//    m = opt_str.value_or(m);
//}

namespace
{
    void seqCleanSeqFileName(QByteArray &filename)
    {
        filename = QDir::cleanPath(filename.toUpper()).toLatin1();
        int loc = filename.indexOf("/SEQUENCERS/");

        if(loc!=-1)
        {
            filename = filename.mid(loc+strlen("/SEQUENCERS/"));
        }
        else
        {
            if(filename.startsWith("SEQUENCERS/")) // TODO: the original was comparing start of a1 with only 3 first letters of SEQUENCERS/, and than skipping the full length ?
            {
                filename.remove(0,strlen("SEQUENCERS/"));
            }
        }
    }

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

    bool loadFrom(BinStore *s, Parser_PlayFx *target)
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
                Parser_PlayFx nt;
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
            seqCleanSeqFileName(nt.name);       // this was done after reading, no reason to not do this now.
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
void serialize(Archive & archive, Parser_PlayFx & m)
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


/* Requires C++17 for std::optional


template<class Archive>
static void serialize(Archive & archive, SeqType & m)
{
    serialize_as_optional(archive,"Name",m.name);
    archive(cereal::make_nvp("Sequencer",m.sequencer_name));
    serialize_as_optional(archive,"SequencerType",m.seq_type);
    serialize_as_optional(archive,"Graphics",m.Lod_Gfx_Names_0);
    serialize_as_optional(archive,"LOD1_Gfx",m.Lod_Gfx_Names_1);
    serialize_as_optional(archive,"LOD2_Gfx",m.Lod_Gfx_Names_2);
    serialize_as_optional(archive,"LOD3_Gfx",m.Lod_Gfx_Names_3);
    serialize_as_optional(archive,"LOD0_Dist",m.LOD0_Dist);
    serialize_as_optional(archive,"LOD1_Dist",m.LOD1_Dist);
    serialize_as_optional(archive,"LOD2_Dist",m.LOD2_Dist);
    serialize_as_optional(archive,"LOD3_Dist",m.LOD3_Dist);
    serialize_as_optional(archive,"VisSphereRadius",m.VisSphereRadius);
    serialize_as_optional(archive,"MaxAlpha",m.maxAlpha);
    serialize_as_optional(archive,"ReverseFadeOutDist",m.reverseFadeOutDist);
    serialize_as_optional(archive,"FadeOutStart",m.FadeOutStart);
    serialize_as_optional(archive,"FadeOutFinish",m.FadeOutFinish);
    serialize_as_optional(archive,"Shadow",m.Shadow);
    serialize_as_optional(archive,"UseShadow",m.UseShadow);
    serialize_as_optional(archive,"TicksToLingerAfterDeath",m.TicksToLingerAfterDeath);
    serialize_as_optional(archive,"TicksToFadeAwayAfterDeath",m.TicksToFadeAwayAfterDeath);
    serialize_as_optional(archive,"ShadowType",m.ShadowType);
    serialize_as_optional(archive,"ShadowTexture",m.ShadowTexture);
    serialize_as_optional(archive,"ShadowQuality",m.ShadowQuality);
    serialize_as_optional(archive,"ShadowSize",m.ShadowSize);
    serialize_as_optional(archive,"ShadowOffset",m.ShadowOffset);
    serialize_as_optional(archive,"Flags",m.Flags);
    serialize_as_optional(archive,"LightAsDoorOutside",m.LightAsDoorOutside);
    archive(cereal::make_nvp("GeomScale",m.GeomScale));
    serialize_as_optional(archive,"GeomScaleMax",m.GeomScaleMax);
    serialize_as_optional(archive,"AnimScale",m.AnimScale);
    serialize_as_optional(archive,"CollisionSize",m.CollisionSize);
    serialize_as_optional(archive,"CollisionOffset",m.CollisionOffset);
    serialize_as_optional(archive,"HasRandomName",m.HasRandomName);
    serialize_as_optional(archive,"Fx",m.Fx0);
    serialize_as_optional(archive,"Fx2",m.Fx1);
    serialize_as_optional(archive,"Fx3",m.Fx2);
    serialize_as_optional(archive,"Fx4",m.Fx3);
    serialize_as_optional(archive,"Fx5",m.Fx4);
    serialize_as_optional(archive,"WorldGroup",m.WorldGroup);
    archive(cereal::make_nvp("MinimumAmbient",m.MinimumAmbient));
    serialize_as_optional(archive,"BoneScaleFat",m.BoneScaleFat);
    serialize_as_optional(archive,"BoneScaleSkinny",m.BoneScaleSkinny);
    serialize_as_optional(archive,"RandomBoneScale",m.RandomBoneScale);
    serialize_as_optional(archive,"NotSelectable",m.NotSelectable);
    serialize_as_optional(archive,"NoCollision",m.NoCollision);
    serialize_as_optional(archive,"CollisionType",m.CollisionType);
    serialize_as_optional(archive,"Bounciness",m.Bounciness);
    serialize_as_optional(archive,"Placement",m.Placement);
    serialize_as_optional(archive,"Selection",m.Selection);
    serialize_as_optional(archive,"ConstantState",m.ConstantState);
    serialize_as_optional(archive,"CapeFile",m.CapeFile);
    serialize_as_optional(archive,"CapeName",m.CapeName);
    serialize_as_optional(archive,"CapeHarnessFile",m.CapeHarnessFile);
    serialize_as_optional(archive,"CapeHarnessName",m.CapeHarnessName);
    serialize_as_optional(archive,"CapeTex1",m.CapeTex1);
    serialize_as_optional(archive,"CapeTex2",m.CapeTex2);
    serialize_as_optional(archive,"CapeColor1",m.CapeColor1);
    serialize_as_optional(archive,"CapeColor2",m.CapeColor2);
    serialize_as_optional(archive,"CapeInnerTex1",m.CapeInnerTex1);
    serialize_as_optional(archive,"CapeInnerTex2",m.CapeInnerTex2);
    serialize_as_optional(archive,"CapeInnerColor1",m.CapeInnerColor1);
    serialize_as_optional(archive,"CapeInnerColor2",m.CapeInnerColor2);
    serialize_as_optional(archive,"ReticleHeightBias",m.ReticleHeightBias);
    serialize_as_optional(archive,"ReticleWidthBias",m.ReticleWidthBias);
}

bool loadFrom(const QString &path, SeqType &target)
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
        arc(cereal::make_nvp("Type", target));
    }
    else if(path.endsWith("bin"))
    {
        if(!ifl.open(QFile::ReadOnly))
        {
            qWarning() << "Failed to open" << path;
            return false;
        }
        std::istringstream istr(ifl.readAll().toStdString());

        cereal::BinaryInputArchive arc(istr);
        arc(cereal::make_nvp("Type", target));
    }
    else
    {
        qWarning() << "Invalid serialized data extension in" <<path;
        return false;
    }

    return true;
}
*/

//! @}
