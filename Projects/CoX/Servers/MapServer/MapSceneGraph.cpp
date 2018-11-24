/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */
#include "MapSceneGraph.h"

#include "GameData/CoHMath.h"
#include "GameData/GameDataStore.h"
#include "EntityStorage.h"
#include "Logging.h"
#include "Common/GameData/Character.h"
#include "Common/Runtime/Prefab.h"
#include "Common/Runtime/SceneGraph.h"
#include "NpcGenerator.h"
#include "MapInstance.h"
#include "GameData/NpcStore.h"

#include "glm/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <QString>
#include <QSet>
#include <memory>

using namespace SEGS;

MapSceneGraph::MapSceneGraph()
{

}
MapSceneGraph::~MapSceneGraph()
{

}

bool MapSceneGraph::loadFromFile(const QString &filename)
{
    m_scene_graph.reset(loadWholeMap(filename));
    if(!m_scene_graph)
        return false;
    for(SceneNode *def : m_scene_graph->all_converted_defs)
    {
        if(def->m_properties)
        {
            m_nodes_with_properties.emplace_back(def);
        }
    }
    return true;
}

void walkSceneNode( SceneNode *self, const glm::mat4 &accumulated, std::function<bool(SceneNode *,const glm::mat4 &)> visit_func )
{
    if(!visit_func(self, accumulated))
        return;

    for(const auto & child : self->m_children)
    {
        glm::mat4 transform(child.m_matrix2);
        transform[3] = glm::vec4(child.m_translation,1);

        walkSceneNode(child.node, accumulated * transform, visit_func);
    }
}

static QString getRandCostumeFromName(const QString &n)
{
    QStringList matching_costumes;
    const NPCStorage &npc_store(getGameData().getNPCDefinitions());
    for(const Parse_NPC &npc : npc_store.m_all_npcs)
    {
        QString name(npc.m_Name);

        // skip some odd cases
        if(name.contains("blimp", Qt::CaseInsensitive))
            continue;
        if(name.contains("monorail", Qt::CaseInsensitive))
            continue;

        if(name.contains(n, Qt::CaseInsensitive))
            matching_costumes.push_back(QString(npc.m_Name));
    }

    if(matching_costumes.isEmpty())
        return "ChessKing";

    int rand_idx = rand() % matching_costumes.size();
    return matching_costumes[rand_idx];
}

static bool checkCostumeExists(const QString &n)
{
    const NPCStorage &npc_store(getGameData().getNPCDefinitions());
    for(const Parse_NPC &npc : npc_store.m_all_npcs)
    {
        if(npc.m_Name == n)
            return true;
    }

    return false;
}

QString getCostumeFromName(const QString &n)
{
    // A lot of costumes are the object name
    if(checkCostumeExists(n))
        return n;

    // Special cases will go here
    // General
    if(n.contains("CarGenerator", Qt::CaseInsensitive)) // must include underscore
        return getRandCostumeFromName("Car_");
    if(n.contains("NPCGenerator", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("DoctorNPC", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("BusinessNPC", Qt::CaseInsensitive))
        return getRandCostumeFromName("maleNPC"); // will include females
    if(n.contains("NPCDrones", Qt::CaseInsensitive))
        return "Police_Drone";
    if(n.contains("MonorailGenerator", Qt::CaseInsensitive))
        return "Car_Monorail";
    if(n.contains("BlimpGenerator", Qt::CaseInsensitive))
        return "Car_Blimp";

    // Outbreak
    if(n.contains("Paragon_SWAT", Qt::CaseInsensitive))
        return getRandCostumeFromName("MaleNPC_230");
    if(n.contains("Lt_MacReady", Qt::CaseInsensitive))
        return "CSE_01";
    if(n.contains("Security_Chief", Qt::CaseInsensitive))
        return getRandCostumeFromName("Security_Chief");

    // Atlas Park
    if(n.contains("FreedomCorp", Qt::CaseInsensitive))
        return getRandCostumeFromName("Freedom_Corp");

    // Most costumes are the object's name without spaces
    // and with Model_ prepended. Assume this as fallback
    QString result = n;
    result = makeReadableName(result);
    result.remove(" ");
    result.prepend("Model_");
    if(checkCostumeExists(result))
        return result;

    // As fallback, always return some costume
    return "ChessPawn";
}

struct NpcCreator
{
    MapInstance *map_instance = nullptr;
    NpcGeneratorStore *generators;
    QSet<QString> m_reported_generators;

    bool checkPersistent(SceneNode *n, const glm::mat4 &v)
    {
        assert(map_instance);
        bool has_npc = false;
        QString persistent_name;
        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName=="PersistentNPC")
                persistent_name = prop.propValue;

            if(prop.propName.toUpper().contains("NPC"))
            {
                qCDebug(logNPCs) << prop.propName << '=' << prop.propValue;
                has_npc = true;
            }
        }

        if(has_npc && map_instance)
        {
            qCDebug(logNPCs) << "Attempting to spawn npc" << persistent_name << "at" << v[3][0] << v[3][1] << v[3][2];
            const NPCStorage & npc_store(getGameData().getNPCDefinitions());
            QString npc_costume_name = getCostumeFromName(persistent_name);
            const Parse_NPC * npc_def = npc_store.npc_by_name(&npc_costume_name);

            if(npc_def)
            {
                int idx = npc_store.npc_idx(npc_def);
                Entity *e = map_instance->m_entities.CreateNpc(getGameData(),*npc_def, idx, 0);
                e->m_char->setName(makeReadableName(persistent_name));
                forcePosition(*e,glm::vec3(v[3]));

                auto valquat = glm::quat_cast(v);
                glm::vec3 angles = glm::eulerAngles(valquat);
                angles.y += glm::pi<float>();
                forceOrientation(*e, angles);
                e->m_motion_state.m_velocity = { 0,0,0 };
            }
        }

        return true;
    }

    bool checkGenerators(SceneNode *n, const glm::mat4 &v)
    {
        if(!generators)
            return false;

        QString generator_type;
        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName=="Generator")
                generator_type = prop.propValue;
        }

        if(generator_type.isEmpty())
            return true;

        if(!generators->m_generators.contains(generator_type))
        {
            qCDebug(logNPCs) << "Adding generator for" << generator_type;

            // Get costume by generator name, includes overrides
            NpcGenerator npcgen = {generator_type, EntType::NPC, {}, {}};

            if(generator_type.contains("Door", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
            if(generator_type.contains("Cardrdr", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
            else if(generator_type.contains("Car_", Qt::CaseInsensitive))
                npcgen.m_type = EntType::CAR;
            else if(generator_type.contains("Nemesis_Drone", Qt::CaseInsensitive))
                npcgen.m_type = EntType::CRITTER;
            else if(generator_type.contains("Door_train", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MAPXFERDOOR;
            else if(generator_type.contains("MonorailGenerator", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MOBILEGEOMETRY;
            else if(generator_type.contains("BlimpGenerator", Qt::CaseInsensitive))
                npcgen.m_type = EntType::MOBILEGEOMETRY;

            generators->m_generators[generator_type] = npcgen;
        }

        generators->m_generators[generator_type].m_initial_positions.push_back(v);
        return true;
    }

    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        checkPersistent(n,v);
        checkGenerators(n,v);
        return true;
    }
};

void MapSceneGraph::spawn_npcs(MapInstance *instance)
{
    NpcCreator creator;
    creator.generators = &instance->m_npc_generators;
    creator.map_instance = instance;
    glm::mat4 initial_pos(1);

    for(auto v : m_scene_graph->refs)
        walkSceneNode(v->node, v->mat, creator);
}

struct SpawnPointLocator
{
    QMultiHash<QString, glm::mat4> *m_targets;
    SpawnPointLocator(QMultiHash<QString, glm::mat4> *targets) :
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName == "SpawnLocation")
            {
                qCDebug(logSpawn) << "Spawner:" << prop.propValue << prop.propertyType;
                m_targets->insert(prop.propValue, v);
                return false;
            }
        }
        return true;
    }
};

QMultiHash<QString, glm::mat4> MapSceneGraph::getSpawnPoints() const
{
    QMultiHash<QString, glm::mat4> res;
    SpawnPointLocator locator(&res);
    for(auto v : m_scene_graph->refs)
        walkSceneNode(v->node, v->mat, locator);

    return res;
}

// TODO: doors should look for, at least, the nearest GotoMap and GotoSpawn properties
// and keep them ready to be used whenever a door is clicked. This code looks at the
// entire scene graph in order to find the nearest SpawnLocation every single time a
// door is clicked.
struct DoorProperties
{
    float distance;
    glm::vec3 location;
    QString gotoSpawn;
};

struct DoorLocator
{
    DoorProperties *m_doorprop;
    DoorLocator(DoorProperties *doorprop) : m_doorprop(doorprop) {}

    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if (!n->m_properties)
            return true;

        // Check the distance to this node, bail if it's farther than what we got.
        glm::vec3 doorloc = glm::vec3(v[3]);
        float doordist = glm::distance(m_doorprop->location, doorloc);
        if (doordist >= m_doorprop->distance)
            return true;

        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if (prop.propName == "GotoSpawn")
            {
                m_doorprop->gotoSpawn = prop.propValue;
                m_doorprop->distance = doordist;
            }
        }
        return true;
    }
};

QString MapSceneGraph::getNearestDoor(glm::vec3 location) const
{
    DoorProperties res;
    res.distance = 10;  // Maximum distance to look for door properties.
    res.location = location;
    DoorLocator locator(&res);

    for (auto v : m_scene_graph->refs)
	{
        walkSceneNode(v->node, v->mat, locator);
    }

    return res.gotoSpawn;
} 

//! @}
