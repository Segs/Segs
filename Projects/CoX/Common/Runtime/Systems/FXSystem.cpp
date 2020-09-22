#include "FXSystem.h"

#include "Runtime/HandleBasedStorage.h"
#include "GameData/GameDataStore.h"
#include "GameData/fx_definitions.h"

#include <QDebug>

namespace FXSystem
{

class FXStorage : public HandleBasedStorage<Data>
{
public:
    static FXStorage &instance()
    {
        static FXStorage instance;
        return instance;
    }
};

namespace
{

    void locateAndAssignFxInputs(const FxInfo *fx,CreationParams &ext_params)
    {
        int input_type=0;
        //auto inputs = ext_params.inputs;
        //TODO: reset transient creation params (locations/targets etc)
        //
        for(const QByteArray &input_name : fx->inputs)
        {
            // select an input_type =>
            //input name in a form : [T_] name [ROOT]
            // T_ -> input name is a bone name or ends with ROOT ? (
            //         ext_params.input[1] has/is sequencer ? (1 : 2) ) : 3
            // no T_ -> input name is a bone name or ends with ROOT ? (
            //         ext_params.input[0] has/is sequencer ? (4 : 5) ) : 3
            // input_type 2 : ext_params.input[inp_idx++].offset = inputs[1].offset;
            // input_type 5 : ext_params.input[inp_idx++].offset = inputs[0].offset;
            // input_type 3 : ext_params.input[inp_idx++] = inputs[src_idx++];
            // input_type 1,4 : TODO
        }
    }
    void onFXFadeOut(Data &dat)
    {
        if(dat.m_state==eFading)
            return;
        //TODO: client-side we would start fading out all dependent/generated objects here
        dat.m_state = eFading;
    }
    void onFXDeath(Data &dat)
    {
        if ( dat.m_state == eAlive )
        {
            //TODO: seath kCondDeath input to allow eDeathEffect to properly fire
            if ( dat.m_fx_info->hasEventsOnDeath )
                dat.m_state = eDeathEffect;
            else
                onFXFadeOut(dat);
        }
    }
} // end of anonymous namespace


Handle create(const QByteArray &name, const CreationParams &params)
{
    GameDataStore &gd(getGameData());
    Data           val;
    val.m_age     = 0.0f;
    FxInfo *info  = gd.getFxInfoByName(name);
    val.m_fx_info = info;
    val.m_params  = params;
    val.m_state   = State::eAlive;
    val.m_name_str_id = gd.getFxNamePackId(name);
    if (!info)
    {
        qDebug() << "Failed to locate Fx description" << QString(name);
        return Handle(0, 0);
    }
    // TODO: info->m_PerformanceRadius could/should be use to cull effects that are farther than any observer ?
    locateAndAssignFxInputs(info, val.m_params);
    val.m_duration = (params.m_duration == 0.0f) ? info->m_LifeSpan : params.m_duration;
    val.m_radius   = params.m_radius;
    val.m_power    = params.m_power;
    val.m_debris   = params.m_debris;
    val.m_net_id   = params.m_net_id;

    auto res = FXStorage::instance().create();
    res.get() = val;
    return res;
}


Data &get(Handle h)
{
    static Data dummy;
    FXStorage::HType dat(h);
    if(!FXStorage::instance().canAccess(h))
    {
        qCritical() << "Dead/uninit handle access";
        return dummy;
    }
    return dat.get();
}

void setSourceLocation(Handle h, glm::vec3 loc)
{
    Data &fx(get(h));
    if(fx.m_params.m_loci.empty())
        fx.m_params.m_loci.resize(1);
    fx.m_params.m_loci[0].m_entity_id=0;
    fx.m_params.m_loci[0].m_offset = loc;
}

void setSourceEntityAndBone(Handle h, int entidx, uint8_t boneidx)
{
    Data &fx(get(h));
    if(fx.m_params.m_loci.empty())
        fx.m_params.m_loci.resize(1);
    fx.m_params.m_loci[0].m_entity_id=entidx;
    fx.m_params.m_loci[0].m_bone_idx=boneidx;
}

void setTargetLocation(Handle h, glm::vec3 loc)
{
    Data &fx(get(h));
    if(fx.m_params.m_loci.empty())
    {
        qCritical("Possibly a wrong order of operations, initialized FX source locus to {0}");
    }
    if(fx.m_params.m_loci.size()<2)
        fx.m_params.m_loci.resize(2);
    //TODO: for now we assume that un-attached NetFx has not been sent

    fx.m_params.m_loci[1].m_entity_id=0;
    fx.m_params.m_loci[1].m_offset=loc;
}
void setTargetEntity(Handle h, int entidx)
{
    Data &fx(get(h));
    if(fx.m_params.m_loci.empty())
    {
        qCritical("Possibly a wrong order of operations, initialized FX source locus to {0}");
    }
    if(fx.m_params.m_loci.size()<2)
        fx.m_params.m_loci.resize(2);

    fx.m_params.m_loci[1].m_entity_id=entidx;
    fx.m_params.m_loci[1].m_bone_idx=2; // default value on the client, not sent over the network
}

bool valid(Handle h)
{
    return FXStorage::instance().canAccess(h);
}

void update(float delta)
{
    //TODO:
    for(Data & d : FXStorage::instance())
    {
        d.m_age += delta * 30;
        if ( d.m_duration != 0.0f && d.m_age > d.m_duration )
        {
            onFXDeath(d);
        }
        //TODO: mark all effects farther than d.m_fx_info.FadeDist as dead/fade?
    }
    //
}

}
