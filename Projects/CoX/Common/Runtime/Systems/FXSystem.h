#pragma once
#include "Common/Runtime/Handle.h"

#include <glm/vec3.hpp>
#include <QString>
#include <vector>

struct FxInfo;

namespace FXSystem
{
// defines a location or entity+bone that is source/target of an effect
struct LocusEntry
{
    glm::vec3 m_offset;
    int       m_entity_id = 0;
    uint8_t   m_bone_idx  = 0;
};

struct CreationParams
{
    //
    float m_duration = 0;
    float m_radius   = 0;
    int   m_debris   = 0;
    int   m_power    = 10;
    int   m_net_id   = 0;
    std::vector<LocusEntry> m_loci; // this should be something like a eastl::fixed_vector<LocusEntry,4>
};
enum State
{
    eAlive=0,
    eDeathEffect=1, // We're waiting to fire the on-effect-death
    eDeath=2,
    eFading=3, //TODO: consider adding eFadedAndGone state in case we want eFading fx to not be destroyed for some time ?
};
class FXStorage;
struct Data
{
    using StorageClass = FXStorage; //tells the handle template to look up

    float          m_age; // measured in ticks ( so time * 30 )
    float          m_duration;
    float          m_radius;
    int            m_power;
    int            m_name_str_id; // index in the string array of the m_fx_info.name;
    int            m_net_id; // only used when FXSystem::data is created 'from' network fx
    int            m_state;
    int            m_debris;

    FxInfo *       m_fx_info;
    CreationParams m_params;
};
using Handle = HandleT<20,12,Data>;


Handle create(const QByteArray &name, const CreationParams &params);
Data & get(Handle h);
bool valid(Handle h);

void setSourceLocation(Handle h, glm::vec3 loc);
void setSourceEntityAndBone(Handle h, int entidx, uint8_t boneidx);
void setTargetLocation(Handle h, glm::vec3 loc);
void setTargetEntity(Handle h, int entidx);

// System update function
void update(float delta);

} // namespace FXSystem

using FXHandle = FXSystem::Handle;
using FXData = FXSystem::Data;
