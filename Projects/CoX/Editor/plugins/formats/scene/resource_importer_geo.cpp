/*************************************************************************/
/*  resource_importer_texture.cpp                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "resource_importer_geo.h"

#include "core/class_db.h"
#include "core/image.h"
#include "core/os/dir_access.h"
#include "core/io/config_file.h"
#include "core/io/image_loader.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/os/mutex.h"
#include "core/project_settings.h"
#include "core/string_utils.h"
#include "editor/service_interfaces/EditorServiceInterface.h"
#include "core/service_interfaces/CoreInterface.h"
#include "scene/resources/texture.h"
#include "scene/3d/spatial.h"
#include "scene/resources/packed_scene.h"

#include "Prefab.h"
#include "RuntimeData.h"
#include "Texture.h"
#include "SceneGraph.h"

#include <QDebug>

struct FileDeleter {
    void operator()(FileAccess *fa) const {
        memdelete(fa);
    }
};
struct SceneGraphInfo {
    Map<SEGS::SceneNode *,int> use_counts;

    enum NodeState
    {
        RootNode=0,
        UsedAsPrefab = 1, // referenced as a child in many places
        InternalNode = 2, // referenced as a child from a single place
    };

    Map<String,SEGS::SceneNode *> calculateUsages(eastl::unique_ptr<SEGS::SceneGraph> &graph)
    {
        Map<String,SEGS::SceneNode *> topLevelNodes;

        for(SEGS::SceneNode *node : graph->all_converted_defs)
        {
            if (node!=nullptr)
            {
                for(auto &child : node->m_children)
                {
                    use_counts[child.node] += 1;
                }

                if (node->m_nest_level != 0) //from included file - not a top level node ?
                {
                    use_counts[node]=0; //just remembering the node, so it won't show in top level ones.
                }
            }
        }

        for(SEGS::SceneNode *node : graph->all_converted_defs)
        {
            if (!use_counts.contains(node))
            {
                if (topLevelNodes.contains(qPrintable(node->m_name)))
                {
                    qDebug()<<"Not returning duplicate node"<<node->m_name;
                    continue;
                }
                topLevelNodes.emplace(qPrintable(node->m_name),node);
            }
        }

        return topLevelNodes;

    }
    NodeState isInternalNode(SEGS::SceneNode *sceneNode)
    {
        int usecount = use_counts.at(sceneNode,-1);
        if (usecount>=0)
        {
            return usecount == 1 ? NodeState::InternalNode : NodeState::UsedAsPrefab;
        }
        return NodeState::RootNode;
    }

    static String GetSafeFilename(const String &filename)
    {
        return filename.replaced('?','_').replaced('+','_');
    }
    static Ref<PackedScene> GetPrefabAsset(const String &tgt_path, const String &model_name)
    {
        String destinationPath = GetSafeFilename(tgt_path + "/" + model_name + ".tscn");
        Error err;
        return dynamic_ref_cast<PackedScene>(ResourceLoader::load(destinationPath, "", true, &err));
    }
};

static String targetDirForNodePrefab(SEGS::SceneNode *n)
{
    auto gs = n->m_geoset_info;
    int idx = gs->geopath.lastIndexOf("geobin");
    String target_directory = qPrintable("Converted/" + gs->geopath.mid(idx));
    return target_directory;
}
Node *convertFromRoot(SceneGraphInfo &sg,SEGS::SceneNode *n)
{
    Node *res = nullptr;
//    auto target_directory = targetDirForNodePrefab(n);
//    if (!m_imported_prefabs.ContainsKey(n))
//    {
//        if (sg.isInternalNode(n) != SceneGraphInfo::InternalNode)
//        {
//            Ref<PackedScene> prefab = sg.GetPrefabAsset(target_directory, qPrintable(n->m_name));
//            if (prefab != nullptr)
//            {
//                res = prefab->instance();
//                return res;
//            }

//        }

//        res = new GameObject(n.m_name);
//        res.isStatic = true;
//        var flgs = GameObjectUtility.GetStaticEditorFlags(res);
//        flgs &= ~StaticEditorFlags.LightmapStatic;

//        GameObjectUtility.SetStaticEditorFlags(res,flgs);

//        n.generated = res;
//        // Convert the whole node.

//        // converting node components
//        convertComponents(n, res);
//        // convert model
//        if (n.m_model != null)
//        {
//            convertModel(n, res, true);
//        }

//        if (!convertChildren(n, res))
//        {
//            GameObject.DestroyImmediate(res);
//            return null;
//        }

//        checkForLodGroup(n, res);
//        updateEditorLayerMarker(res);

//    }

//    switch (sg.isInternalNode(n))
//    {
//    case NodeState.InternalNode:
//        break;
//    case NodeState.RootNode:
//    case NodeState.UsedAsPrefab:
//        if (!m_imported_prefabs.ContainsKey(n))
//        {
//            var res2 = CreatePrefabFromModel(target_directory, res);
//            m_imported_prefabs[n] = res2;
//        }

//        if (m_imported_prefabs[n] == null)
//            return null;

//        var res3 = (GameObject) PrefabUtility.InstantiatePrefab(m_imported_prefabs[n]);
//        if (n.generated == null)
//            n.generated = res3;
//        return res3;
//    default:
//        throw new ArgumentOutOfRangeException();
//    }
    assert(false);
    return res;
}
Node *EditorSceneImporterCoHGeo::import_scene(se_string_view p_path, uint32_t p_flags, int p_bake_fps,
                                              Vector<String> *r_missing_deps, Error *r_error)
{

    // Check if the selected file is correctly located in the hierarchy, just to make sure.
    SceneGraphInfo sg_info;
    auto idx = StringUtils::find_last(p_path,"geobin");
    if (String::npos == idx)
    {

        if (r_error)
            *r_error = ERR_CANT_OPEN;
        PLUG_FAIL_V_MSG(nullptr, "The given source file is not located in geobin/ folder.");
    }
    auto rd(getRuntimeData());
    if (!rd.m_ready)
    {
        // we expect coh data to live under res://coh_data
        String basepath = "./coh_data";
        rd.prepare(basepath.c_str());
    }

    m_scene_graph.reset(SEGS::loadWholeMap(String(p_path).c_str()));
    auto top_nodes = sg_info.calculateUsages(m_scene_graph);
    if (!top_nodes.empty())
    {
        bool some_prefabs_were_missing = false;
        for(auto &pair : top_nodes)
        {
            Node *top_level = convertFromRoot(sg_info,pair.second);
            if (top_level == nullptr)
                some_prefabs_were_missing = true;
        }

//        for(auto &pair : sg_info.m_imported_prefabs)
//        {
//            if (!top_nodes.contains(pair.first))
//                GameObject.DestroyImmediate(pair.Value); // remove all non-top-level prefabs from scene.
//        }

//        if (some_prefabs_were_missing == false)
//            createTopNodesInstances(top_nodes);
//        else
//        {
//            string saved_ones = String.Join("\n", m_created_prefabs);
//            ctx.LogImportWarning(String.Format(
//                "The following prefab assets were missing and were created, please retry: {0}",
//                saved_ones));
//        }
    }
    if (r_error)
        *r_error = ERR_CANT_OPEN;
    return nullptr;
}
Ref<Animation> EditorSceneImporterCoHGeo::import_animation(se_string_view p_path, uint32_t p_flags, int p_bake_fps) {
    // we don't support this YET
    return Ref<Animation>();
}
uint32_t EditorSceneImporterCoHGeo::get_import_flags() const {

    return IMPORT_SCENE | IMPORT_ANIMATION; //  | IMPORT_MATERIALS_IN_INSTANCES for overrides?
}

void EditorSceneImporterCoHGeo::get_extensions(Vector<String> &r_extensions) const {
    // CoH scene graph files have this extension.
    r_extensions.emplace_back("bin");
}
