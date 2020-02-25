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
#include "scene/3d/light.h"

#include "Prefab.h"
#include "RuntimeData.h"
#include "Texture.h"
#include "SceneGraph.h"

#include "glm/gtx/matrix_decompose.hpp"

#include <QDebug>

static Spatial *convertFromRoot(struct SceneGraphInfo &sg, Spatial *parent, SEGS::SceneNode *n);
struct FileDeleter {
    void operator()(FileAccess *fa) const {
        memdelete(fa);
    }
};
struct SceneGraphInfo {
    Map<SEGS::SceneNode *,int> use_counts;
    Map<SEGS::SceneNode *,Ref<PackedScene>> m_imported_prefabs;
    Spatial *root;
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
Color fromGLM(glm::vec4 v) {
    return Color(v.r,v.g,v.b,v.a);
}
static void convertLightComponent(SEGS::SceneNode *n, Spatial *res)
{
    OmniLight *light_node = memnew(OmniLight);
    if (n->m_light->is_negative)
    {
        light_node->set_negative(true);
    }
    // since light object is disabled by default, we don't want to disable ourselves,
    // since we have light probes
    // Light objects are put into seperate layer, to allow fast sphere collider lookups.
    //light_node->set_layer_mask();
    //lobj.layer = LayerMask.NameToLayer("OmniLights");
    res->add_child(light_node);

    light_node->set_name("OmniLight");
    light_node->set_owner(res);
    light_node->set_color(fromGLM(n->m_light->color));
    light_node->set_param(Light::PARAM_RANGE,n->m_light->range);

    //light.lightmapBakeType = LightmapBakeType.Realtime;
    //light.cullingMask = ~(1 << 9); // Don't light the layer 9 - Editor object
    //light.enabled = true;
    //light.intensity = 4;

    //SphereCollider sp_c = lobj.AddComponent<SphereCollider>();
    //sp_c.radius = light.range;
    //sp_c.isTrigger = true;

    //LightProbeGroup lpb=lobj.AddComponent<LightProbeGroup>();
    //lpb.probePositions = createProbes(3, 2, light.range);
}
static void convertComponents(SEGS::SceneNode *n, Spatial *res)
{
    if (n->m_light != nullptr)
    {
        convertLightComponent(n, res);
    }

//    var sup = getOrCreateComponent<NodeMods>(res);
//    if (n.m_properties != null)
//    {
//        if (sup.Props.Properties == null && n.m_properties.Count != 0)
//            sup.Props.Properties = n.m_properties;
//    }

//    if (n.sound_info != null)
//    {
//        Debug.Log("Has sound properties");
//        SoundInfo coh_snd = n.sound_info;
//        var snd = getOrCreateComponent<AudioSource>(res);
//        snd.maxDistance = coh_snd.radius;
//        snd.minDistance = coh_snd.radius - coh_snd.ramp_feet;
//        snd.volume = coh_snd.vol / 255.0f;
//        snd.clip = AssetDatabase.LoadAssetAtPath<AudioClip>(coh_snd.name);
//        if (null == snd.clip)
//        {
//            Debug.LogWarningFormat("Failed to locate sound asset {0}", coh_snd.name);
//        }
//    }

//    if (n.m_editor_beacon != null)
//    {
//        if (n.m_model != null)
//        {
//            Debug.Log("Not adding beacon sphere to a node with model.");
//        }
//        else
//        {
//            var pm = GameObject.CreatePrimitive(PrimitiveType.Sphere);
//            pm.name = n.m_editor_beacon.name;
//            MeshRenderer mr = pm.GetComponent<MeshRenderer>();
//            mr.receiveShadows = false;
//            pm.transform.localScale = Vector3.one * n.m_editor_beacon.amplitude;
//            pm.transform.SetParent(res.transform);
//            pm.layer = 9;
//            mr.sharedMaterial = AssetDatabase.LoadAssetAtPath<Material>("Assets/Materials/Beacon.mat");
//        }
//    }
}
static void convertModel(SEGS::SceneNode *n, Spatial *res, bool convert_editor_markers)
{
//    string mesh_path;
//    string model_path;
//    int obj_lib_idx;

//    Model mdl = n.m_model;
//    ModelModifiers model_trick = mdl.trck_node;
//    mesh_path = mdl.geoset.full_geo_path;
//    obj_lib_idx = mesh_path.IndexOf("object_library");
//    if (obj_lib_idx != -1)
//        mesh_path = "Assets/Meshes/" + mesh_path.Substring(obj_lib_idx);
//    model_path = GetSafeFilename(mesh_path + "/" + Path.GetFileNameWithoutExtension(mdl.name) + ".asset");

//    MeshFilter mf = res.AddComponent<MeshFilter>();
//    MeshRenderer ren = res.AddComponent<MeshRenderer>();
//    var sup = res.AddComponent<ModelNodeMods>();
//    if (model_trick != null)
//    {
//        sup.ModelMod = model_trick;
//        applyTricks(res,model_trick);
//    }

//    mf.sharedMesh = AssetDatabase.LoadAssetAtPath<Mesh>(model_path);
//    if (model_trick != null)
//    {
//        ren.shadowCastingMode = ShadowCastingMode.Off;
//        if (model_trick._TrickFlags.HasFlag(TrickFlags.NoDraw))
//        {
//            ren.receiveShadows = false;
//            res.tag = "NoDraw";
//            res.layer = 9;
//        }

//        if (model_trick._TrickFlags.HasFlag(TrickFlags.EditorVisible))
//        {
//            ren.receiveShadows = false;
//            res.layer = 9;
//        }

//        if (model_trick._TrickFlags.HasFlag(TrickFlags.CastShadow))
//        {
//            ren.receiveShadows = false;
//            ren.shadowCastingMode = ShadowCastingMode.ShadowsOnly;
//        }

//        if (model_trick._TrickFlags.HasFlag(TrickFlags.ParticleSys))
//        {
//            Debug.Log("Not converting particle sys:" + mdl.name);
//            return;
//        }
//    }
//    if (!mdl.geoset.data_loaded)
//    {
//        mdl.geoset.LoadData();
//        if (mdl.geoset.subs.Count != 0)
//            mdl.geoset.createEngineModelsFromPrefabSet();
//    }
//    if (mf.sharedMesh == null)
//    {
//        UnityModel res_static;
//        RuntimeData rd = RuntimeData.get();
//        if (!rd.s_coh_model_to_engine_model.TryGetValue(mdl, out res_static))
//            return;
//        if (res_static == null)
//            return;
//        SEGSRuntime.Tools.EnsureDirectoryExists(mesh_path);
//        AssetDatabase.CreateAsset(res_static.m_mesh, model_path);
//        AssetDatabase.SaveAssets();
//        mf.sharedMesh = AssetDatabase.LoadAssetAtPath<Mesh>(model_path);
//    }

//    convertMaterials(ren, mdl, res);
    qDebug("Not converting models yet");
}
Transform fromGLM(glm::mat3 m,glm::vec3 t) {
    Transform res (
        m[0][0],m[0][1],m[0][2],
        m[1][0],m[1][1],m[1][2],
        m[2][0],m[2][1],m[2][2],
        t.x,t.y,t.z
        );
    return res;
}
static String targetDirForNodePrefab(SEGS::SceneNode *n)
{
    auto gs = n->m_geoset_info;
    int idx = gs->geopath.lastIndexOf("geobin");
    String target_directory = qPrintable("Converted/" + gs->geopath.mid(idx));
    return target_directory;
}
static bool convertChildren(SceneGraphInfo &sg,SEGS::SceneNode *n, Spatial *res)
{
    bool all_ok = true;
    for(const SEGS::SceneNodeChildTransform &child : n->m_children)
    {
        Spatial *go = convertFromRoot(sg, res, child.node);
        all_ok &= (go != nullptr);
        if (go != nullptr)
        {
            go->set_transform(fromGLM(child.m_matrix2,child.m_translation));
        }
    }

    return all_ok;
}
Spatial *convertFromRoot(SceneGraphInfo &sg, Spatial *parent, SEGS::SceneNode *n)
{
    Spatial *res = nullptr;
    if (!sg.m_imported_prefabs.contains(n))
    {
        if (sg.isInternalNode(n) == SceneGraphInfo::UsedAsPrefab)
        {
            auto target_directory = targetDirForNodePrefab(n);
            Ref<PackedScene> prefab = sg.GetPrefabAsset(target_directory, qPrintable(n->m_name));
            if (prefab != nullptr)
            {
                sg.m_imported_prefabs[n] = prefab; // record for later use.
                auto prefab_res = prefab->instance();
                assert(object_cast<Spatial>(prefab_res)!=nullptr);
                return (Spatial *)prefab_res;
            }
        }
        res = memnew(Spatial);
        res->set_name(qPrintable(n->m_name));
        parent->add_child(res);
        res->set_owner(sg.root);
        n->m_engine_node = res;
//        res.isStatic = true;
//        var flgs = GameObjectUtility.GetStaticEditorFlags(res);
//        flgs &= ~StaticEditorFlags.LightmapStatic;

        // Convert the whole node.

        // converting node components
        convertComponents(n, res);
        // convert model
        if (n->m_model != nullptr)
        {
            convertModel(n, res, true);
        }

        if (!convertChildren(sg,n, res))
        {
            memdelete(res);
            return nullptr;
        }

//        checkForLodGroup(n, res);
//        updateEditorLayerMarker(res);

    }

    switch (sg.isInternalNode(n))
    {
    case SceneGraphInfo::InternalNode:
        break;
    case SceneGraphInfo::RootNode:
    case SceneGraphInfo::UsedAsPrefab:
    {
        if (!sg.m_imported_prefabs.contains(n))
        {
            //            var res2 = CreatePrefabFromModel(target_directory, res);
            //            m_imported_prefabs[n] = res2;
        }

        if (sg.m_imported_prefabs[n] == nullptr)
            return res; //TODO: handle this case, when we need the prefab for node and it's missing

        auto res3 = sg.m_imported_prefabs[n]->instance();
        if (n->m_engine_node == nullptr)
            n->m_engine_node = res3;
        return (Spatial *)res3;
    }
    default:
        ; //        throw new ArgumentOutOfRangeException();
    }

    return res;
}
Node *EditorSceneImporterCoHGeo::import_scene(StringView p_path, uint32_t p_flags, int p_bake_fps,
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
    String fs_path(p_path);
    fs_path.replace("res:/", ProjectSettings::get_singleton()->get_resource_path());

    auto &rd(getRuntimeData());
    if (!rd.m_ready)
    {
        // we expect coh data to live under res://coh_data
        String basepath = ProjectSettings::get_singleton()->get_resource_path()+"/coh_data/";
        ;
        if(!rd.prepare(basepath.c_str()))
        {
            PLUG_FAIL_V_MSG(nullptr, "The required bin files are missing ?");
        }
    }

    m_scene_graph.reset(SEGS::loadWholeMap(fs_path.c_str()));
    Spatial* layer_root = memnew(Spatial);
    sg_info.root = layer_root;
    auto top_nodes = sg_info.calculateUsages(m_scene_graph);
    if (!top_nodes.empty())
    {
        bool some_prefabs_were_missing = false;
        for(auto &pair : top_nodes)
        {
            Node *top_level = convertFromRoot(sg_info,layer_root, pair.second);
            if (top_level == nullptr)
                some_prefabs_were_missing = true;
        }

        //TODO: remove all non-top-level prefabs from scene.
//        for(auto &pair : sg_info.m_imported_prefabs)
//        {
//            if (!top_nodes.contains(pair.first))
//                GameObject.DestroyImmediate(pair.Value);
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
        *r_error = OK;
    return layer_root;
}
Ref<Animation> EditorSceneImporterCoHGeo::import_animation(StringView p_path, uint32_t p_flags, int p_bake_fps) {
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
