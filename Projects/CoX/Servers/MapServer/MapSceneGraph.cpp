/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */
#include "MapSceneGraph.h"

#include "GameData/CoHMath.h"
#include "MapServerData.h"
#include "SceneGraph.h"
#include "EntityStorage.h"
#include "Logging.h"
#include "Common/NetStructures/Character.h"
#include "NpcGenerator.h"
#include "MapInstance.h"
#include "NpcStore.h"

#include "glm/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <QSet>

namespace
{
    PrefabStore g_prefab_store;
}
MapSceneGraph::MapSceneGraph()
{

}
MapSceneGraph::~MapSceneGraph()
{

}


bool MapSceneGraph::loadFromFile(const QString &filename)
{
    m_scene_graph.reset(new SceneGraph);
    LoadingContext ctx;
    ctx.m_target = m_scene_graph.get();
    int geobin_idx= filename.indexOf("geobin");
    int maps_idx = filename.indexOf("maps");
    ctx.m_base_path = filename.mid(0, geobin_idx);
    g_prefab_store.prepareGeoLookupArray(ctx.m_base_path);
    QString upcase_city = filename;
    upcase_city.replace("city","City");
    upcase_city.replace("zones","Zones");
    bool res = loadSceneGraph(upcase_city.mid(maps_idx), ctx, g_prefab_store);
    if (!res)
        return false;

    for(const auto &def : m_scene_graph->all_converted_defs)
    {
        if(def->properties)
        {
            m_nodes_with_properties.emplace_back(def);
        }
    }
    return res;
}
void walkSceneNode( SceneNode *self, const glm::mat4 &accumulated, std::function<bool(SceneNode *,const glm::mat4 &)> visit_func )
{
    if (!visit_func(self, accumulated))
        return;
    for(const auto & child : self->children)
    {
        glm::mat4 transform(child.m_matrix2);
        transform[3] = glm::vec4(child.m_translation,1);

        walkSceneNode(child.node, accumulated * transform, visit_func);
    }
}
static QString convertNpcName(const QString &n)
{
    if(n.contains("Sergeant_Hicks"))
    {
        return "Model_SergeantHicks";
    }
    if (n.contains("Officer_Parks"))
    {
        return "Model_OfficerParks";
    }
    if (n.contains("Doctor_Miller"))
    {
        return "Model_DoctorMiller";
    }
    if (n.contains("Detective_Wright"))
    {
        return "Model_DetectiveWright";
    }
    if (n.contains("Officer_Flint"))
    {
        return "Model_OfficerFlint";
    }
    if(n.contains("Paragon_SWAT"))
    {
        return "CSE_01";
    }
    if(n.contains("Professor_Hoffman"))
    {
        return "Model_ProfessorHoffman";
    }
    if(n.contains("Lt_MacReady"))
    {
        return "Model_SusanDavies";
    }
    return "ChessPawn";
}

struct NpcCreator
{
    MapInstance *map_instance = nullptr;
    NpcGeneratorStore *generators;
    QSet<QString> m_reported_generators;

    bool checkPersistant(SceneNode *n, const glm::mat4 &v)
    {
        bool has_npc = false;
        QString persistant_name;
        for (GroupProperty_Data &prop : *n->properties)
        {
            if(prop.propName=="PersistentNPC")
            {
                persistant_name = prop.propValue;
            }
            if (prop.propName.contains("NPC", Qt::CaseInsensitive))
            {
                qDebug() << prop.propName << '=' << prop.propValue;
                has_npc = true;
            }
        }
        if(has_npc && map_instance)
        {
            qDebug() << "Attempt to spawn npc" << persistant_name << "at"<<v[3][0] << v[3][1] << v[3][2];
            const NPCStorage & npc_store(map_instance->serverData().getNPCDefinitions());
            QString npc_costume_name = convertNpcName(persistant_name);
            const Parse_NPC * npc_def = npc_store.npc_by_name(&npc_costume_name);
            if (npc_def)
            {
                int idx = npc_store.npc_idx(npc_def);
                Entity *e = map_instance->m_entities.CreateNpc(*npc_def, idx, 0);
                e->m_entity_data.m_pos = glm::vec3(v[3]);
                auto valquat = glm::quat_cast(v);

                glm::vec3 angles = glm::eulerAngles(valquat);
                angles.y += glm::pi<float>();
                valquat = glm::quat(angles);
                e->m_char->setName(npc_costume_name);
                e->m_direction = valquat;
                e->m_entity_data.m_orientation_pyr = {angles.x,angles.y,angles.z};
                e->vel = { 0,0,0 };
            }
        }
        return true;
    }
    bool checkGenerators(SceneNode *n, const glm::mat4 &v)
    {
        if(!generators)
            return false;
        QString generator_type;
        for (GroupProperty_Data &prop : *n->properties)
        {
            if(prop.propName=="Generator")
            {
                generator_type = prop.propValue;
            }
        }
        if(generator_type.isEmpty())
            return true;
        if(!generators->m_generators.contains(generator_type))
        {
            if(!m_reported_generators.contains(generator_type))
            {
                qDebug() << "Missing generator for"<<generator_type;
                m_reported_generators.insert(generator_type);
            }
            return true;
        }
        generators->m_generators[generator_type].initial_positions.push_back(v);
        return true;
    }
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if (!n->properties)
            return true;
        checkPersistant(n,v);
        checkGenerators(n,v);
        return true;
    }
};
void MapSceneGraph::spawn_npcs(MapInstance *instance)
{

/*  glm::vec3 gm_loc = sess.m_ent->m_entity_data.m_pos;
    if (!npc_def)
    {
        sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for:" + parts[1], &sess);
        return;
    }
    glm::vec3 offset = glm::vec3{ 2,0,1 };
    e->m_entity_data.m_pos = gm_loc + offset;
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1").arg(e->m_idx), &sess);
    */
    NpcCreator creator;
    creator.generators = &instance->m_npc_generators;
    creator.map_instance = instance;
    glm::mat4 initial_pos(1);
    for(auto v : m_scene_graph->refs)
    {
        walkSceneNode(v->node, v->mat, creator);
    }
}

struct SpawnPointLocator
{
    QString m_kind;
    std::vector<glm::mat4> *m_targets;
    SpawnPointLocator(const QString &kind,std::vector<glm::mat4> *targets) :
        m_kind(kind),
        m_targets(targets)
    {}
    bool operator()(SceneNode *n, const glm::mat4 &v)
    {
        if (!n->properties)
            return true;
        for (GroupProperty_Data &prop : *n->properties)
        {
            if(prop.propName=="SpawnLocation" && 0==prop.propValue.compare(m_kind))
            {
                m_targets->emplace_back(v);
                return false; //
            }
        }
        return true;
    }
};

std::vector<glm::mat4> MapSceneGraph::spawn_points(const QString &kind) const
{
    std::vector<glm::mat4> res;
    SpawnPointLocator locator(kind,&res);
    for(auto v : m_scene_graph->refs)
    {
        walkSceneNode(v->node, v->mat, locator);
    }
    return res;
}

//! @}
