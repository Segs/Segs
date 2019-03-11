/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */
#include "MapSceneGraph.h"

#include "WorldSimulation.h"
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

struct EnemySpawnPointLocator
{
    QMultiHash<QString, glm::mat4> *m_targets;
    EnemySpawnPointLocator(QMultiHash<QString, glm::mat4> *targets) :
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if(!n->m_properties)
            return true;

        for (GroupProperty_Data &prop : *n->m_properties)
        {

            if(prop.propName == "EncounterPosition")
            {
                qCDebug(logPlayerSpawn) << "Encounter:" << prop.propValue << prop.propertyType;
                m_targets->insert(prop.propValue, v);
                return false;
            }
        }
        return true;
    }
};

void MapSceneGraph::visitNodesFromRoots(std::function<bool (SceneNode *, const glm::mat4 &)> visit_func)
{
    for (auto v : m_scene_graph->refs)
    {
        walkSceneNode(v->node, v->mat, visit_func);
    }
}


//! @}
