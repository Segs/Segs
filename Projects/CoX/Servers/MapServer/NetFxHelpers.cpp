#include "NetFxHelpers.h"

#include "GameData/Entity.h"
#include "GameData/GameDataStore.h"
#include "HandleBasedStorage.h"

using NetFxStore = HandleBasedStorage<NetFx>;
//TODO: for now we assume that attached NetFx has been sent

//"FX/WORLD/CITY/STREETSTEAM01.FX"
NetFxHandle createNetFx(const QString &fx_name)
{
    GameDataStore &gd(getGameData());

    NetFxHandle res = NetFxStore::instance().create();
    NetFx &fx(lookup(res));

    fx.command = 4; // create ?
    int true_idx = gd.net_fx_handle(fx_name);
    fx.handle = true_idx+1;
    fx.client_timer = 1.0f; // trigger after a time
    fx.duration = 0.0f; // use default duration
    fx.power = 10; // 10 is default, so we could probably send 0 here
    fx.debris = 0;

    return res;
}
void attachToEntity(NetFxHandle h,Entity *tgt)
{
    NetFx &fx(lookup(h));
    fx.net_id = h.idx;
    tgt->m_net_fx.emplace_back(h);
    tgt->m_pchar_things = true;
}

void setSourceLocation(NetFxHandle h, glm::vec3 loc)
{
    NetFx &fx(lookup(h));
    assert(fx.net_id==0);
    fx.origin.type_is_location = true;
    fx.origin.pos = loc;
}
void setSourceEntityAndBone(NetFxHandle h, int entidx, uint8_t boneidx)
{
    NetFx &fx(lookup(h));
    assert(fx.net_id==0);
    fx.origin.type_is_location = false;
    fx.origin.ent_idx = entidx;
    fx.bone_id = boneidx;

}

void setTargetLocation(NetFxHandle h, glm::vec3 loc)
{
    NetFx &fx(lookup(h));
    //TODO: for now we assume that un-attached NetFx has not been sent
    assert(fx.net_id==0);
    fx.target.type_is_location = true;
    fx.target.pos = loc;
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



