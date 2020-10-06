#include "particlesys_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "particle_definitions.h"
#include "DataStorage.h"

#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <QtCore/QDebug>
namespace
{
std::vector<glm::vec3> convertToVec3Vector(const std::vector<float>& src) {
    std::vector<glm::vec3> res;
    assert((src.size() % 3) == 0);
    res.reserve(src.size()/3);
    for(size_t i=0; i<src.size(); i+=3) {
        res.emplace_back(src[i],src[i+1],src[i+2]);
    }
    return res;
}
bool loadFrom(BinStore * s, ParticleSystemInfo * target)
{
    bool ok = true;
    ok &= s->read(target->m_Name);
    ok &= s->read(target->m_FrontOrLocalFacing);
    ok &= s->read(target->m_WorldOrLocalPosition);
    ok &= s->read(target->m_TimeToFull);
    ok &= s->read(target->m_KickStart);
    ok &= s->read(target->m_NewPerFrame);
    ok &= s->read(target->m_Burst);
    ok &= s->read(target->m_BurbleAmplitude);
    ok &= s->read(target->m_BurbleType);
    ok &= s->read(target->m_BurbleFrequency);
    ok &= s->read(target->m_BurbleThreshold);
    ok &= s->read(target->m_MoveScale);
    ok &= s->read(target->m_EmissionType);
    std::vector<float> val;
    ok &= s->read(val);
    target->m_EmissionStartJitter = convertToVec3Vector(val);
    val.clear();

    ok &= s->read(target->m_EmissionRadius);
    ok &= s->read(target->m_EmissionHeight);
    ok &= s->read(target->m_Spin);
    ok &= s->read(target->m_SpinJitter);
    ok &= s->read(target->m_OrientationJitter);
    ok &= s->read(target->m_Magnetism);
    ok &= s->read(target->m_Gravity);
    ok &= s->read(target->m_KillOnZero);
    ok &= s->read(target->m_Terrain);

    ok &= s->read(val);
    target->m_InitialVelocity = convertToVec3Vector(val);
    val.clear();

    ok &= s->read(val);
    target->m_InitialVelocityJitter = convertToVec3Vector(val);
    val.clear();
    ok &= s->read(target->m_VelocityJitter);
    ok &= s->read(target->m_TightenUp);
    ok &= s->read(target->m_SortBias);
    ok &= s->read(target->m_Drag);
    ok &= s->read(target->m_Stickiness);
    ok &= s->read(target->m_Alpha);
    ok &= s->read(target->m_ColorChangeType);
    for(int i=0; i<5; ++i)
        ok &= s->read(target->m_StartColor[i].startcolor); //
    for(int i=1; i<5; ++i)
        ok &= s->read(target->m_StartColor[i].startTime);
    for(int i=0; i<5; ++i) // same target offset in CoH template, overwriting
        ok &= s->read(target->m_StartColor[i].startcolor);
    for(int i=0; i<5; ++i)
        ok &= s->read(target->m_StartColor[i].startTime);
    ok &= s->read(target->m_FadeInBy);
    ok &= s->read(target->m_FadeOutStart);
    ok &= s->read(target->m_FadeOutBy);
    ok &= s->read(target->m_DieLikeThis);
    ok &= s->read(target->m_DeathAgeToZero);
    ok &= s->read(target->m_StartSize);
    ok &= s->read(target->m_StartSizeJitter);
    ok &= s->read(target->m_Blend_mode);
    ok &= s->read(target->particleTexture[0].m_TextureName);
    ok &= s->read(target->particleTexture[1].m_TextureName);
    ok &= s->read(target->particleTexture[0].m_TexScroll);
    ok &= s->read(target->particleTexture[1].m_TexScroll);
    ok &= s->read(target->particleTexture[0].m_TexScrollJitter);
    ok &= s->read(target->particleTexture[1].m_TexScrollJitter);
    ok &= s->read(target->particleTexture[0].m_AnimFrames);
    ok &= s->read(target->particleTexture[1].m_AnimFrames);
    ok &= s->read(target->particleTexture[0].m_AnimPace);
    ok &= s->read(target->particleTexture[1].m_AnimPace);
    ok &= s->read(target->particleTexture[0].m_AnimType);
    ok &= s->read(target->particleTexture[1].m_AnimType);
    ok &= s->read(target->m_EndSize);
    ok &= s->read(target->m_ExpandRate);
    ok &= s->read(target->m_ExpandType);
    ok &= s->read(target->m_StreakType);
    ok &= s->read(target->m_StreakScale);
    ok &= s->read(target->m_StreakOrient);
    ok &= s->read(target->m_StreakDirection);
    ok &= s->read(target->m_VisRadius);
    ok &= s->read(target->m_VisDist);
    ok &= s->read(target->m_Flags); // eFlags
    ok &= s->prepare_nested();

    return ok && s->end_encountered();

}
}
template<class Archive>
void serialize(Archive & archive, ColorFx & m)
{
    archive(cereal::make_nvp("startcolor",m.startcolor),cereal::make_nvp("startTime",m.startTime));
}

template<class Archive>
void serialize(Archive &archive, ParticleTextureData & m)
{
    archive(cereal::make_nvp("TextureName",m.m_TextureName));
    archive(cereal::make_nvp("TexScroll",m.m_TexScroll));
    archive(cereal::make_nvp("TexScrollJitter",m.m_TexScrollJitter));
    archive(cereal::make_nvp("AnimFrames",m.m_AnimFrames));
    archive(cereal::make_nvp("AnimPace",m.m_AnimPace));
    archive(cereal::make_nvp("AnimType",m.m_AnimType));
}
template<class Archive>
void serialize(Archive & archive, ParticleSystemInfo & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("FrontOrLocalFacing",m.m_FrontOrLocalFacing));
    archive(cereal::make_nvp("WorldOrLocalPosition",m.m_WorldOrLocalPosition));
    archive(cereal::make_nvp("TimeToFull",m.m_TimeToFull));
    archive(cereal::make_nvp("KickStart",m.m_KickStart));
    archive(cereal::make_nvp("NewPerFrame",m.m_NewPerFrame));
    archive(cereal::make_nvp("Burst",m.m_Burst));
    archive(cereal::make_nvp("BurbleAmplitude",m.m_BurbleAmplitude));
    archive(cereal::make_nvp("BurbleType",m.m_BurbleType));
    archive(cereal::make_nvp("BurbleFrequency",m.m_BurbleFrequency));
    archive(cereal::make_nvp("BurbleThreshold",m.m_BurbleThreshold));
    archive(cereal::make_nvp("MoveScale",m.m_MoveScale));
    archive(cereal::make_nvp("EmissionType",m.m_EmissionType));
    archive(cereal::make_nvp("EmissionStartJitter",m.m_EmissionStartJitter));
    archive(cereal::make_nvp("EmissionRadius",m.m_EmissionRadius));
    archive(cereal::make_nvp("EmissionHeight",m.m_EmissionHeight));
    archive(cereal::make_nvp("Spin",m.m_Spin));
    archive(cereal::make_nvp("SpinJitter",m.m_SpinJitter));
    archive(cereal::make_nvp("OrientationJitter",m.m_OrientationJitter));
    archive(cereal::make_nvp("Magnetism",m.m_Magnetism));
    archive(cereal::make_nvp("Gravity",m.m_Gravity));
    archive(cereal::make_nvp("KillOnZero",m.m_KillOnZero));
    archive(cereal::make_nvp("Terrain",m.m_Terrain));
    archive(cereal::make_nvp("InitialVelocity",m.m_InitialVelocity));
    archive(cereal::make_nvp("InitialVelocityJitter",m.m_InitialVelocityJitter));
    archive(cereal::make_nvp("VelocityJitter",m.m_VelocityJitter));
    archive(cereal::make_nvp("TightenUp",m.m_TightenUp));
    archive(cereal::make_nvp("SortBias",m.m_SortBias));
    archive(cereal::make_nvp("Drag",m.m_Drag));
    archive(cereal::make_nvp("Stickiness",m.m_Stickiness));
    archive(cereal::make_nvp("Alpha",m.m_Alpha));
    archive(cereal::make_nvp("ColorChangeType",m.m_ColorChangeType));
    archive(cereal::make_nvp("StartColor",m.m_StartColor));
    archive(cereal::make_nvp("FadeInBy",m.m_FadeInBy));
    archive(cereal::make_nvp("FadeOutStart",m.m_FadeOutStart));
    archive(cereal::make_nvp("FadeOutBy",m.m_FadeOutBy));
    archive(cereal::make_nvp("DieLikeThis",m.m_DieLikeThis));
    archive(cereal::make_nvp("DeathAgeToZero",m.m_DeathAgeToZero));
    archive(cereal::make_nvp("StartSize",m.m_StartSize));
    archive(cereal::make_nvp("StartSizeJitter",m.m_StartSizeJitter));
    archive(cereal::make_nvp("Blend_mode",m.m_Blend_mode));
    archive(cereal::make_nvp("frames",m.particleTexture));
    archive(cereal::make_nvp("EndSize",m.m_EndSize));
    archive(cereal::make_nvp("ExpandRate",m.m_ExpandRate));
    archive(cereal::make_nvp("ExpandType",m.m_ExpandType));
    archive(cereal::make_nvp("StreakType",m.m_StreakType));
    archive(cereal::make_nvp("StreakScale",m.m_StreakScale));
    archive(cereal::make_nvp("StreakOrient",m.m_StreakOrient));
    archive(cereal::make_nvp("StreakDirection",m.m_StreakDirection));
    archive(cereal::make_nvp("VisRadius",m.m_VisRadius));
    archive(cereal::make_nvp("VisDist",m.m_VisDist));
    archive(cereal::make_nvp("Flags",m.m_Flags)); // eFlags
}
template<class Archive>
static void save(Archive & archive, Parse_AllPSystems const & m)
{
    archive(m.m_Systems);
}

template<class Archive>
static void load(Archive & archive, Parse_AllPSystems & m)
{
    archive(m.m_Systems);
    for(size_t idx = 0,total=m.m_Systems.size(); idx<total; ++idx)
    {
        QString name_helper = m.m_Systems[idx].m_Name.toLower();

        if(m.m_NameToIdx.contains(name_helper)) {
            qWarning() << "Duplicate Particle system named" << name_helper;
        }
        m.m_NameToIdx[name_helper] = idx;
    }
}

bool loadFrom(BinStore * s, Parse_AllPSystems * target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        s->prepare();
        if("System"==_name) {
            ParticleSystemInfo nt;
            ok &= loadFrom(s,&nt);
            cleanupPSystemName(nt.m_Name); // normalize the name by removing the leading /FX/ path spec
            if(target->m_NameToIdx.contains(nt.m_Name.toLower())) {
                qWarning() << "Duplicate Particle system named" << nt.m_Name;
            }
            target->m_NameToIdx[nt.m_Name.toLower()] = target->m_Systems.size();
            target->m_Systems.push_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;

}

void saveTo(const Parse_AllPSystems & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"ParticleSystems",baseName,text_format);
}
