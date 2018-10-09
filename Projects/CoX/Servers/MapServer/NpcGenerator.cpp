#include "NpcGenerator.h"

#include "MapInstance.h"
#include "GameData/GameDataStore.h"
#include "GameData/NpcStore.h"
#include "NetStructures/Character.h"

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
    for(const glm::mat4 &v : initial_positions)
    {
        int idx = npc_store.npc_idx(npc_def);
        Entity *e = map_instance->m_entities.CreateGeneric(getGameData(), *npc_def, idx, 0, type);
        e->m_char->setName(makeReadableName(costume_name));
        forcePosition(*e, glm::vec3(v[3]));

        auto valquat = glm::quat_cast(v);
        glm::vec3 angles = glm::eulerAngles(valquat);
        angles.y += glm::pi<float>();
        forceOrientation(*e, angles);
        e->m_motion_state.m_velocity = { 0,0,0 };
    }
}

void NpcGeneratorStore::generate(MapInstance *instance)
{
    for(NpcGenerator &gen : m_generators)
    {
        gen.generate(instance);
    }
}
