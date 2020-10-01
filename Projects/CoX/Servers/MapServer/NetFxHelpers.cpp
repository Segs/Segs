#include "NetFxHelpers.h"

#include "GameData/Entity.h"
#include "GameData/GameDataStore.h"
#include "Runtime/Systems/FXSystem.h"
#include "HandleBasedStorage.h"

class NetFxStore : public HandleBasedStorage<NetFx>
{
public:
    static NetFxStore &instance()
    {
        static NetFxStore instance;
        return instance;
    }
};

namespace
{

void setupNetFxTarget(const FXSystem::LocusEntry & entry,NetFxTarget &tgt)
{
    if(entry.m_entity_id!=0)
    {
        tgt.type_is_location = false;
        tgt.ent_idx = entry.m_entity_id;
        tgt.bone_idx = entry.m_bone_idx ? entry.m_bone_idx : 2;
    }
    else
    {
        tgt.type_is_location = true;
        tgt.pos = entry.m_offset;
    }
}
}
//TODO: for now we assume that attached NetFx has been sent

//"FX/WORLD/CITY/STREETSTEAM01.FX"
NetFxHandle createNetFx(FxHandle from_fx)
{
    FXSystem::Data &data(FXSystem::get(from_fx));

    NetFxHandle res = NetFxStore::instance().create();
    NetFx &fx(lookup(res));
    fx.command = 4; // create ?
    fx.handle = data.m_name_str_id;
    fx.client_timer = 1.0f; // trigger after a time
    fx.m_parent = from_fx;
    fx.duration = data.m_duration; // use default duration
    fx.power = data.m_power; // 10 is default, so we could probably send 0 here
    fx.debris = data.m_debris;
    // converting loci
    assert(data.m_params.m_loci.size()>=2);
    const FXSystem::LocusEntry & entry(data.m_params.m_loci[0]);
    setupNetFxTarget(entry,fx.origin);
    const FXSystem::LocusEntry & entry2(data.m_params.m_loci[1]);
    setupNetFxTarget(entry2,fx.target);
    return res;
}
void attachToEntity(NetFxHandle h,Entity *tgt)
{
    NetFx &fx(lookup(h));
    if(!FXSystem::valid(fx.m_parent))
    {
        qCritical("Parent effect is no longer/not yet a valid FX, cannot attach");
        return;
    }
    fx.net_id = h.idx;
    fx.m_ref_count++;
    tgt->m_net_fx.emplace_back(h);
    tgt->m_entity_update_flags.setFlag(Entity::FX,true);
}

NetFx &lookup(NetFxHandle handle)
{
    static NetFx dummy;
    if(!NetFxStore::instance().canAccess(handle))
    {
        qCritical() << "Invalid handle accessed";
        return dummy;
    }
    return NetFxStore::instance().access(handle);
}

void release(NetFxHandle handle)
{
    NetFxStore::instance().destroy(handle);
}

/**
 * @brief updateFromParent change the state of NetFx based on it's parent Fx state
 * @param h - handle to NetFx instance to be updated
 * @return true if the given netfxhandle should be erased from the entity
 */
bool updateNetFxFromParent(NetFxHandle h)
{
    //TODO: effects are destroyed immediately when they enter eFading state, this will likely cut the client-side
    //effect 'short'

    NetFx &fx(lookup(h));
    fx.m_ref_count++;

    if( (fx.command & 8) || fx.destroy_next_update )
    {
        fx.m_ref_count--;
        if(fx.m_ref_count==0)
            release(h);
        return true;
    }

    if(!FXSystem::valid(fx.m_parent))
    {
        fx.command |= 8; // destroy
        return false;
    }
    FXSystem::Data &parent(FXSystem::get(fx.m_parent));
    if(parent.m_state == FXSystem::eFading)
    {
        fx.command |= 8;
        return false;
    }
    // Update the fx ??
    const FXSystem::LocusEntry & entry(parent.m_params.m_loci[0]);
    setupNetFxTarget(entry,fx.origin);
    const FXSystem::LocusEntry & entry2(parent.m_params.m_loci[1]);
    setupNetFxTarget(entry2,fx.target);
    if(parent.m_duration==0.0f && parent.m_age>0)
    {
        fx.destroy_next_update = true;
    }
    return false;
}

/// Removes netfx that were previously not marked as used
/// Marks all netfx as unused.
/// The following calls to updateFromParent will mark active fx handles as such.
void updateNetFx()
{
    auto &store(NetFxStore::instance());
    std::vector<NetFxHandle> to_remove;
    for(NetFx iter : store)
        if(iter.m_ref_count==0)
            to_remove.emplace_back(store.handle_for_entry(iter));
    for(NetFxHandle h : to_remove)
        store.destroy(h);
    for(NetFx &entry : NetFxStore::instance())
        entry.m_ref_count=0;
}
