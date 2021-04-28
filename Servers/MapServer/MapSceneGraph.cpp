/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
#include "Components/Logging.h"
#include "Common/GameData/Character.h"
#include "Common/Runtime/Prefab.h"
#include "Common/Runtime/SceneGraph.h"
#include "NpcGenerator.h"
#include "MapInstance.h"
#include "GameData/NpcStore.h"
#include "CritterGenerator.h"
#include "Common/GameData/map_definitions.h"
#include "Common/GameData/spawn_definitions.h"

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

bool MapSceneGraph::loadFromFile(const QByteArray &filename)
{
    QFSWrapper wrap;
    m_scene_graph.reset(loadWholeMap(&wrap, filename));
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

glm::vec3 FindOrientation(glm::mat4 v)
{
    auto valquat = glm::quat_cast(v);
    glm::vec3 angles = glm::eulerAngles(valquat);
    angles.y += glm::pi<float>();

    return angles;
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

    /* Special cases will go here
        For now, only monorail and blimp generation remains here
        This will be moved into the vehicle-related Lua framework when managers
        are later created to deal with them.
    */
    if(n.contains("NPCDrones", Qt::CaseInsensitive))
            return "Police_Drone";  // Only handling drones lining sidewalk in Outbreak; will become obsolete.
    if(n.contains("MonorailGenerator", Qt::CaseInsensitive))
        return "Car_Monorail";
    if(n.contains("BlimpGenerator", Qt::CaseInsensitive))
        return "Car_Blimp";

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

        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName=="PersistentNPC")
            {
                SpawnerNode snNode;
                QString persName = QString::fromStdString(prop.propValue.toStdString());
                persName = makeReadableName(persName);
                if(persName.contains(" "))
                    persName.replace(" ", "_");

                snNode.m_name = persName;         // Persistents use node name to store the NPC name instead
                qCDebug(logNpcSpawn) << "Persistent parsed: " << persName;
                snNode.m_position = glm::vec3(v[3]);
                snNode.m_rotation = FindOrientation(v);
                map_instance->m_map_scenegraph->m_persNodes.push_back(snNode);
            }
        }

        return true;
    }

    bool checkCanSpawns(SceneNode *n, const glm::mat4 &v)
    {
        assert(map_instance);

        for (GroupProperty_Data &prop : *n->m_properties)
        {
            if(prop.propName.contains("CanSpawn1"))
            {
                //qCDebug(logNPCs) << " CanSpawn loc: " << v[3][0] << v[3][1] << v[3][2];
                SpawnerNode snNode;
                snNode.m_name = n->m_name;
                snNode.m_position = glm::vec3(v[3]);
                snNode.m_rotation = FindOrientation(v);

                for (uint i = 0; i < n->m_children.size(); i++)
                {
                    SpawnerNode snChildNode;

                    snChildNode.m_name = n->m_children.at(i).node->m_name;

                    // handle position
                    glm::mat4 transform(n->m_children.at(i).m_matrix2);
                    transform[3] = glm::vec4(n->m_children.at(i).m_translation,1);
                    transform = v * transform;
                    glm::vec4 pos4 {0,0,0,1};
                    pos4 = transform * pos4;
                    glm::vec3 pos3 = glm::vec3(pos4);
                    snChildNode.m_position = pos3;

                    // handle rotation
                    // This isn't always right but it's as close as anything I've seen.
                    snChildNode.m_rotation = snNode.m_rotation + n->m_children.at(i).m_pyr;

                    snNode.m_markers.push_back(snChildNode);
                }
                map_instance->m_map_scenegraph->m_csNodes.push_back(snNode);
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
            {
                QString propValue = prop.propValue;
                SpawnerNode snNode;
                snNode.m_position = glm::vec3(v[3]);
                snNode.m_rotation = FindOrientation(v);

                if(propValue.contains("CarGenerator", Qt::CaseSensitive))
                {
                    snNode.m_name = n->m_name;
                    map_instance->m_map_scenegraph->m_carNodes.push_back(snNode);
                }
                else if (propValue.contains("NPCGenerator", Qt::CaseSensitive))
                {
                    snNode.m_name = n->m_name;
                    map_instance->m_map_scenegraph->m_npcNodes.push_back(snNode);
                }
                else
                {
                    generator_type = prop.propValue;
                }
            }
        }

        if(generator_type.isEmpty())
            return true;

        if(!generators->m_generators.contains(generator_type))
        {
            // Get costume by generator name, includes overrides
            NpcGenerator npcgen = {generator_type, EntType::NPC, {}, {}};

            if(generator_type.contains("Door", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
            if(generator_type.contains("Cardrdr", Qt::CaseInsensitive))
                npcgen.m_type = EntType::DOOR;
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

        checkPersistent(n,v);           // locates persistent NPCs; exposes to Lua
        checkCanSpawns(n,v);            // locates encounters with CanSpawn/Spawndef; exposes to Lua
        checkGenerators(n,v);           // locates and spawns doors, monos, trains; exposes cars, npcs to Lua
        return true;
    }
};

void MapSceneGraph::spawn_npcs(MapInstance *instance)
{
    NpcCreator creator;
    creator.generators = &instance->m_npc_generators;
    creator.map_instance = instance;
    glm::mat4 initial_pos(1);

    for(auto v : m_scene_graph->roots)
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
                //qCDebug(logPlayerSpawn) << "Spawner:" << prop.propValue << prop.propertyType;
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
    for(auto v : m_scene_graph->roots)
        walkSceneNode(v->node, v->mat, locator);

    return res;
}


struct MapXferLocator
{
    QHash<QString, MapXferData> *m_targets;
    MapXferLocator(QHash<QString, MapXferData> *targets):
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {

        if (!n->m_properties)
        {
            for (auto &child : n->m_children)
            {
                bool found_map_transfer = false;
                if (child.node->m_properties != nullptr)
                {
                    MapXferData map_transfer = MapXferData();
                    // Probably haven't processed the map swap node yet, so add it and handle later
                    for (GroupProperty_Data &prop : *child.node->m_properties)
                    {
                        if (prop.propName == "GotoSpawn")
                        {
                            map_transfer.m_target_spawn_name = prop.propValue;
                            found_map_transfer = true;
                        }
                        if (prop.propName == "GotoMap")
                        {
                            map_transfer.m_target_map_name = prop.propValue.split('.')[0];
                            // Assume that if there's a GotoMap, that it's for a map xfer.
                            // TODO: Change the transfer type detection to something less ambiguous if possible.
                            map_transfer.m_transfer_type = MapXferType::ZONE;
                            found_map_transfer = true;
                        }
                    }
                    if (found_map_transfer)
                    {
                        map_transfer.m_node_name = child.node->m_name;

                        // get position
                        glm::mat4 transform(child.m_matrix2);
                        transform[3] = glm::vec4(child.m_translation,1);
                        transform = v * transform;
                        glm::vec4 pos4 {0,0,0,1};
                        pos4 = transform * pos4;
                        glm::vec3 pos3 = glm::vec3(pos4);

                        map_transfer.m_position = pos3;
                        m_targets->insert(map_transfer.m_node_name, map_transfer);
                        return false;
                    }
                }
            }
        }

        return true;
    }
};

QHash<QString, MapXferData> MapSceneGraph::get_map_transfers() const
{
    QHash<QString, MapXferData> res;
    MapXferLocator locator(&res);
    for (auto v : m_scene_graph->roots)
    {
        walkSceneNode(v->node, v->mat, locator);
    }
    return res;
}

//! @}
