#include "NpcGenerator.h"

#include "MapInstance.h"
#include "GameData/GameDataStore.h"
#include "GameData/NpcStore.h"
#include "GameData/Character.h"

#include <QRegularExpression>

QString makeReadableName(QString &name)
{
    // remove filepaths and extensions then replace underscores with spaces
    return name.remove(QRegularExpression(".*\\\\")).remove(QRegularExpression("\\..*")).replace("_"," ");
}

void NpcGenerator::generate(MapInstance *map_instance)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());
    const Parse_NPC * npc_def = npc_store.npc_by_name(&costume_name);
    if (!npc_def)
        return;
    NetFx fx;
    GameDataStore &gd(getGameData());
    fx.command = 4; // create ?
    int true_idx = gd.net_fx_handle("FX/WORLD/CITY/STREETSTEAM01.FX");
    fx.handle = true_idx+1;
    fx.client_timer = 1.0f; // trigger after a time
    fx.duration = 0.0f; // use default duration
    fx.power = 10; // 10 is default, so we could probably send 0 here
    for(const glm::mat4 &v : initial_positions)
    {
        int idx = npc_store.npc_idx(npc_def);
        Entity *e = map_instance->m_entities.CreateGeneric(getGameData(), *npc_def, idx, 0,type);
        e->m_char->setName(makeReadableName(costume_name));
        forcePosition(*e, glm::vec3(v[3]));
        auto valquat = glm::quat_cast(v);

        glm::vec3 angles = glm::eulerAngles(valquat);
        angles.y += glm::pi<float>();
        forceOrientation(*e, angles);
        e->m_motion_state.m_velocity = { 0,0,0 };

        if(costume_name.contains("door", Qt::CaseInsensitive))
        {
            e->m_is_fading = false;
            e->translucency = 0.0f;
        }
        else if(true_idx)
        {
            fx.net_id++;
            fx.debris = 0;
            fx.origin.type_is_location = true;
            fx.origin.pos = glm::vec3(v[3]);
            fx.target.type_is_location = true;
            fx.target.pos = glm::vec3(v[3])+glm::vec3(0,1,0);
            e->m_net_fx.emplace_back(fx);
            e->m_pchar_things = true;
        }
    }
}

void NpcGeneratorStore::generate(MapInstance *instance)
{
    for(NpcGenerator &gen : m_generators)
    {
        gen.generate(instance);
    }
}
