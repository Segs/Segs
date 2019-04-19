#include "NpcGenerator.h"

#include "MapInstance.h"
#include "MapSceneGraph.h"
#include "GameData/GameDataStore.h"
#include "GameData/NpcStore.h"
#include "GameData/Character.h"

#include <QRegularExpression>

QString makeReadableName(QString &name)
{
    // remove filepaths and extensions then replace underscores with spaces; handles SG contacts
    QString tempName = name.remove(QRegularExpression(".*\\\\")).remove(QRegularExpression("\\..*"));
    if(tempName.contains("SuperGroupContacts"))
        tempName.remove("SuperGroupContacts/");
    return tempName;
}

void NpcGenerator::generate(MapInstance *map_instance)
{
    const NPCStorage & npc_store(getGameData().getNPCDefinitions());

    for(const glm::mat4 &v : m_initial_positions)
    {
        if(m_generator_name.contains("NPCGenerator", Qt::CaseInsensitive))
        {
            // too many pedestrians slows the client down, let's spawn a portion only
            if(rand() % 7)
                continue;
        }
        else if(m_generator_name.contains("CarGenerator", Qt::CaseInsensitive))
        {
           if(rand() % 3)
               continue;
        }

        QString npc_costume_name = getCostumeFromName(m_generator_name);

        const Parse_NPC * npc_def = npc_store.npc_by_name(&npc_costume_name);
        if(!npc_def)
            continue;

        int idx = npc_store.npc_idx(npc_def);
        Entity *e = map_instance->m_entities.CreateGeneric(getGameData(), *npc_def, idx, 0, m_type);
        e->m_char->setName(makeReadableName(npc_costume_name));
        forcePosition(*e, glm::vec3(v[3]));

        auto valquat = glm::quat_cast(v);
        glm::vec3 angles = glm::eulerAngles(valquat);
        angles.y += glm::pi<float>();
        forceOrientation(*e, angles);
        e->m_motion_state.m_velocity = { 0,0,0 };


        if(m_generator_name.contains("door", Qt::CaseInsensitive))
        {
            e->m_is_fading = false;
            e->translucency = 0.0f;
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
