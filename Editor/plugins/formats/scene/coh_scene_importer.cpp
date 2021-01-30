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

#include "coh_scene_importer.h"

#include "core/class_db.h"
#include "core/image.h"
#include "core/io/config_file.h"
#include "core/io/image_loader.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/dir_access.h"
#include "core/os/mutex.h"
#include "core/project_settings.h"
#include "core/resource/resource_manager.h"
#include "core/service_interfaces/CoreInterface.h"
#include "core/string_utils.h"
#include "editor/service_interfaces/EditorServiceInterface.h"
#include "scene/3d/instantiation.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"

#include "scene/resources/packed_scene.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/scene_library.h"
#include "scene/resources/texture.h"

#include "Common/GameData/scenegraph_serializers.h"
#include "Common/Runtime/Model.h"
#include "GameData/trick_definitions.h"
#include "Prefab.h"
#include "RuntimeData.h"
#include "SceneGraph.h"
#include "Texture.h"

#include "glm/gtx/matrix_decompose.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>




struct FileIOWrap : public QIODevice
{
    explicit FileIOWrap(FileAccess *fa) : m_fa(fa) {}
    bool isSequential() const override { return false; }
    /*qint64 pos() const
    {
        return m_fa->get_position();
    }*/
    qint64 size() const override { return m_fa->get_len(); }
    bool seek(qint64 pos) override
    {
        QIODevice::seek(pos);
        m_fa->seek(pos);
        return this->pos()==pos;
    }
    qint64 bytesAvailable() const override { return m_fa->get_len() - pos(); }
    void   close() override
    {
        QIODevice::close();
        if(m_fa)
            m_fa->close();
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override { return m_fa->get_buffer((uint8_t *)data, maxlen); }
    qint64 writeData(const char *data, qint64 len) override
    {
        auto p = pos();
        m_fa->store_buffer((const uint8_t*)data, len);
        return pos() - p;
    }

public:
    mutable FileAccessRef<> m_fa;
};
struct SE_FSWrapper : public FSWrapper
{
    static Set<String> missing_files;
    SE_FSWrapper() { missing_files.clear(); }
    QIODevice * open(const QString &path, bool read_only, bool text_only) override
    {
        FileAccess *wrap(FileAccess::open(qPrintable(path), read_only ? FileAccess::READ : FileAccess::READ_WRITE));
        if (!wrap)
        {

            missing_files.insert(qPrintable("res://"+path.mid(path.lastIndexOf("coh_data"))));
            return nullptr;
        }
        auto res = new FileIOWrap(wrap);
        res->open((read_only ? QIODevice::ReadOnly : QIODevice::ReadWrite) |
                  (text_only ? QIODevice::Text : QIODevice::NotOpen));
        return res;
    }
    bool exists(const QString &path) override
    {
        return DirAccess::exists(qPrintable(path))||FileAccess::exists(qPrintable(path));
    }
    QStringList dir_entries(const QString &path) override
    {
        DirAccessRef da(DirAccess::open(qPrintable(path)));
        if(!da)
            return {}; //TODO: report missing directory?
        da->list_dir_begin();
        QStringList res;
        String item;
        while (!(item = da->get_next()).empty())
        {

            if (item == "." || item == "..")
                continue;
            res.push_back(StringUtils::from_utf8(item));
        }
        return res;
    }
};
Set<String> SE_FSWrapper::missing_files;

struct SceneGraphInfo
{
    Vector<Node3D *> root_stack;
    HashSet<String>   missing_imported_geosets;
    Ref<SceneLibrary> lib;
    String m_root_path;

    static String GetSafeFilename(const String &filename) { return filename.replaced('?', '_').replaced('+', '_'); }

    static Ref<PackedScene> GetPrefabAsset(const String &tgt_path, const String &model_name)
    {
        String destinationPath = SceneGraphInfo::GetSafeFilename(tgt_path + "/" + model_name + ".scn");
        Error err;
        return dynamic_ref_cast<PackedScene>(gResourceManager().load(destinationPath, "", true, &err));
    }
};
Color fromGLM(glm::vec4 v)
{
    return Color(v.r,v.g,v.b);
}
static Node* convertFromRoot(SceneGraphInfo& sg, SEGS::SceneNode* n);

static void convertLightComponent(SEGS::SceneNode *n, Node3D *res)
{
    OmniLight3D *light_node = memnew(OmniLight3D);
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
    light_node->set_color(Color(fromGLM(n->m_light->color)));
    light_node->set_param(Light3D::PARAM_RANGE,n->m_light->range);

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
static void convertComponents(SEGS::SceneNode *n, Node3D *res)
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

    if (n->sound_info)
    {
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
    }

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
static void convertModel(Node *owner, SEGS::Model *mdl, Node3D *res, HashSet<String> &missing_models)
{
    ModelModifiers *model_trick = mdl->trck_node;
    String meshlib_path = PathUtils::get_base_dir(mdl->geoset->geopath.data());
    String geoset_file(PathUtils::get_basename(StringView(mdl->geoset->geopath.data())));
    String mesh_name = qPrintable(mdl->name);


    size_t obj_lib_idx = meshlib_path.find("object_library");
    if (obj_lib_idx != String::npos)
        meshlib_path = "coh_data/" + meshlib_path.substr(obj_lib_idx);
    SE_FSWrapper wrap;
    MeshInstance3D *mi = memnew(MeshInstance3D);
    res->add_child(mi);
    mi->set_owner(owner);

    String meshlib_res_path = "res://"+meshlib_path+"/"+geoset_file+".geo";
    if (!FileAccess::exists(meshlib_res_path))
    {
        mi->set_mesh(make_ref_counted<CubeMesh>());
        missing_models.emplace(eastl::move(meshlib_res_path));
        return;
    }
    SEGS::toSafeModelName(mesh_name.data(),mesh_name.size());
    String mesh_res_path = "res://"+meshlib_path+"/"+mesh_name+".mesh";
    if (!FileAccess::exists(mesh_res_path))
    {
        mi->set_mesh(make_ref_counted<CubeMesh>());
        missing_models.insert(mesh_res_path);
        return;
    }
    Ref<Mesh> loaded_mesh = dynamic_ref_cast<Mesh>(gResourceManager().load(mesh_res_path));
    if (!FileAccess::exists(mesh_res_path))
    {
        mi->set_mesh(make_ref_counted<CubeMesh>());
        missing_models.insert(mesh_res_path);
        return;
    }
    mi->set_mesh(loaded_mesh);

//    var sup = res.AddComponent<ModelNodeMods>();
//    mf.sharedMesh = AssetDatabase.LoadAssetAtPath<Mesh>(model_path);
    if (model_trick != nullptr)
    {
        auto shadow_mode=GeometryInstance::SHADOW_CASTING_SETTING_OFF;
//        ren.shadowCastingMode = ShadowCastingMode.Off;
//        if (model_trick._TrickFlags.HasFlag(TrickFlags.NoDraw))
//        {
//            ren.receiveShadows = false;
//            res.tag = "NoDraw";
//            res.layer = 9;
        //        sup.ModelMod = model_trick;
        //        applyTricks(res,model_trick);
//        }

//        if (model_trick._TrickFlags.HasFlag(TrickFlags.EditorVisible))
//        {
//            ren.receiveShadows = false;
//            res.layer = 9;
//        }

        if (model_trick->_TrickFlags&TrickFlags::CastShadow)
        {
            shadow_mode = GeometryInstance::SHADOW_CASTING_SETTING_SHADOWS_ONLY;
//            ren.receiveShadows = false;
        }

//        if (model_trick._TrickFlags.HasFlag(TrickFlags.ParticleSys))
//        {
//            Debug.Log("Not converting particle sys:" + mdl.name);
//            return;
//        }
        mi->set_cast_shadows_setting(shadow_mode);
    }
}
static Transform fromGLM(glm::mat3 m,glm::vec3 t) {
    Transform res (
        m[0][0],m[0][1],m[0][2],
        m[1][0],m[1][1],m[1][2],
        m[2][0],m[2][1],m[2][2],
        t.x,t.y,t.z
        );
    return res;
}
static Transform fromGLM(glm::mat4 m) {
    Transform res (
        m[0][0],m[0][1],m[0][2],
        m[1][0],m[1][1],m[1][2],
        m[2][0],m[2][1],m[2][2],
        m[3][0],m[3][1],m[3][2]
        );
    return res;
}
static bool convertChildren(SceneGraphInfo &sg,SEGS::SceneNode *n, Node3D *res)
{
    bool all_ok = true;
    for(const SEGS::SceneNodeChildTransform &child : n->m_children)
    {
        Node* go;
        if(child.node->m_engine_node)
            go = static_cast<Node *>(child.node->m_engine_node);
        else
            go = convertFromRoot(sg, child.node);

        all_ok &= (go != nullptr);
        if (go != nullptr)
        {
            res->add_child(go);
            go->set_owner(sg.root_stack.back());

            //            Quat quat2 = Quat(Vector3(-child.m_pyr.x, child.m_pyr.y * 180 / M_PI, child.m_pyr.z * 180 /
            //            M_PI));
            Quat qPitch = Quat(Vector3(-1,0,0),child.m_pyr.x);
            Quat qYaw   = Quat(Vector3(0,1,0),child.m_pyr.y);
            Quat qRoll  = Quat(Vector3(0,0,1),child.m_pyr.z);
            Quat rotQuat = qYaw * qPitch * qRoll;
            //rotQuat.set_euler_yxz(Vector3(child.m_pyr.x,child.m_pyr.y,child.m_pyr.z));
            Transform res2;
            res2.set_basis(rotQuat);
            res2.origin = Vector3(child.m_translation.x,child.m_translation.y,child.m_translation.z);
//            Transform res2 = fromGLM(child.m_matrix2,child.m_translation);
//
            ((Node3D*)go)->set_transform(res2);
//            ((Node3D *)go)->set_rotation(Vector3(child.m_pyr.x,child.m_pyr.y,child.m_pyr.z));
            //            ((Node3D
            //            *)go)->set_translation(Vector3(child.m_translation.x,child.m_translation.y,child.m_translation.z));
            //((Node3D *)go)->set_transform(fromGLM(child.m_matrix2,child.m_translation));
        }
    }

    return all_ok;
}
static Node3D *convertInternal(SceneGraphInfo &sg, SEGS::SceneNode *n)
{
    Node3D *res = memnew(Node3D);
    if (sg.root_stack.empty() || n->m_use_count>1)
    {
        sg.root_stack.emplace_back(res);
    }

    res->set_name(qPrintable(n->m_name));
    // Convert the whole node.

    // converting node components
    convertComponents(n, res);
    // convert model
    if (n->m_model != nullptr)
    {
        convertModel(sg.root_stack.back(), n->m_model, res, sg.missing_imported_geosets);
    }

    if (!convertChildren(sg,n, res))
    {
        memdelete(res);
        return nullptr;
    }
//        checkForLodGroup(n, res);
//        updateEditorLayerMarker(res);
    return res;
}
static bool autoGen(const QString &n)
{
    if(!n.startsWith("grp") || !n.back().isDigit())
        return false;
    return true;
}
static Node *convertFromRoot(SceneGraphInfo &sg, SEGS::SceneNode *n)
{
    LibraryItemHandle lib_id = sg.lib->find_item_by_name(n->m_name.data());
    if(lib_id!=-1)
    {
        Ref<PackedScene> sc(sg.lib->get_item_scene(lib_id));
        auto             instance_that = sc->instance(GEN_EDIT_STATE_INSTANCE);
        //        LibraryEntryInstance *instance_that=memnew(LibraryEntryInstance);
        //        instance_that->set_library_path(sg.lib->get_path());
        //        instance_that->set_entry(n->m_name.data());
        //        return sg.lib->get_item_scene(lib_id)->instance();
        instance_that->set_filename(sg.lib->get_path() + "::" + StringUtils::num(lib_id));
        instance_that->set_name(String(String::CtorSprintf(),"%p_%s",instance_that,n->m_name.data()));
        return instance_that;
    }
    Node *res = convertInternal(sg, n);
    if(!res)
        return nullptr;

    if (n->m_use_count <= 1 && autoGen(n->m_name) && n->m_nest_level <= 0)
    {
        assert(!sg.root_stack.empty());
        return res;
    }
    // This node is used in multiple places in our current scene-graph, convert it to prefab.
    SceneLibrary::Item new_prefab;
    new_prefab.name  = n->m_name.data();
    new_prefab.scene = make_ref_counted<PackedScene>();
    new_prefab.scene->pack(res);
    lib_id = sg.lib->add_item(eastl::move(new_prefab));
    Ref<PackedScene>  sc(sg.lib->get_item_scene(lib_id));
    sc->set_path(sg.lib->get_path() + "::" + StringUtils::num(lib_id));
    sc->set_subindex(lib_id);
    auto instance_that = sc->instance(GEN_EDIT_STATE_INSTANCE);
    //    LibraryEntryInstance *instance_that =memnew(LibraryEntryInstance);
    //    instance_that->set_library_path(sg.lib->get_path());
    //    instance_that->set_entry(n->m_name.data());
    instance_that->set_name(String(String::CtorSprintf(),"%p_%s",instance_that,n->m_name.data()));
    instance_that->set_filename(sg.lib->get_path() + "::" + StringUtils::num(lib_id));
    // We leave library as null to refer to ourselves.
    // Null library will get hopefully resolved by LibraryEntryInstance::add_child_notify
    res->queue_delete();
    //res = sg.lib->get_item_scene(h)->instance();
    sg.root_stack.pop_back();
    return instance_that;
}

const char *CoHSceneLibrary::get_importer_name() const
{
    return "coh_scenelib";
}

const char *CoHSceneLibrary::get_visible_name() const
{
    return "CoH Scene Library";
}

void CoHSceneLibrary::get_recognized_extensions(Vector<String> &p_extensions) const
{
    // CoH scene graph files have this extension.
    p_extensions.emplace_back("bin");
}

bool CoHSceneLibrary::can_import(StringView path) const
{
    return path.contains("geobin/");
}

StringName CoHSceneLibrary::get_save_extension() const
{
    return "scenelib";
}

StringName CoHSceneLibrary::get_resource_type() const
{
    return "SceneLibrary";
}

StringName CoHSceneLibrary::get_preset_name(int /*p_idx*/) const
{

    return StringName();
}

bool CoHSceneLibrary::get_option_visibility(const StringName & /*p_option*/,
                                            const HashMap<StringName, Variant> &/*p_options*/) const
{
    return true;
}
static bool nonAutoNodeName(const SEGS::SceneNode * /*node*/)
{
    return true;
}
static bool nodeIsMultiInstantiated(const SEGS::SceneNode *node)
{
    return node->m_use_count>1 || nonAutoNodeName(node);
}
static Ref<SceneLibrary> build_scene_library(SEGS::SceneGraph &m_scene_graph, Vector<String> &missing_resources,
                                             StringView p_source_file)
{
    HashSet<String> exported_scene_roots;
    SceneGraphInfo  sg;
    for (const auto &r : m_scene_graph.roots)
    {
        exported_scene_roots.insert(r->node->m_name.data());
    }
    sg.lib = make_ref_counted<SceneLibrary>();
    sg.lib->set_path(p_source_file);
    QFileInfo fi(QByteArray::fromRawData(p_source_file.data(),p_source_file.size()));
    sg.lib->set_name(fi.baseName().toLatin1().data());
    for (SEGS::SceneNode *r : m_scene_graph.all_converted_defs)
    {
        if (!nodeIsMultiInstantiated(r) && !exported_scene_roots.contains(r->m_name.data()))
            continue;

        Node *top_level = convertFromRoot(sg, r);
        assert(sg.root_stack.empty());
        if (sg.lib->find_item_by_name(r->m_name.data()) == LibraryItemHandle(-1))
        {
            SceneLibrary::Item dat;
            dat.scene = make_ref_counted<PackedScene>();
            dat.scene->pack(top_level);
            dat.name = r->m_name.data();
            sg.lib->add_item(eastl::move(dat));
        }
        else
            memdelete(top_level);
    }
    missing_resources.insert(missing_resources.end(), sg.missing_imported_geosets.begin(),
                             sg.missing_imported_geosets.end());
    return eastl::move(sg.lib);
}

Error CoHSceneLibrary::import(StringView p_source_file, StringView p_save_path,
                              const HashMap<StringName, Variant> &p_options, Vector<String> &r_missing_deps,
                              Vector<String> *r_platform_variants, Vector<String> *r_gen_files, Variant *r_metadata)
{
    using namespace StringUtils;

    SE_FSWrapper se_wrap;
    SceneGraphInfo sg_info;
    auto idx = StringUtils::find_last(p_source_file, "geobin");

    // Check if the selected file is correctly located in the hierarchy, just to make sure.
    if (String::npos == idx)
    {
        PLUG_FAIL_V_MSG(ERR_CANT_OPEN, "The given source file is not located in geobin/ folder.");
    }
    String fs_path(p_source_file);
    fs_path.replace("res:/", ProjectSettings::get_singleton()->get_resource_path());

    auto& rd(getRuntimeData());
    if (!rd.m_ready)
    {
        // we expect coh data to live under res://coh_data
        String basepath = ProjectSettings::get_singleton()->get_resource_path() + "/coh_data/";

        if (!rd.prepare(&se_wrap, basepath.c_str()))
        {
            PLUG_FAIL_V_MSG(ERR_FILE_MISSING_DEPENDENCIES, "The required bin files are missing ?");
        }
    }

    eastl::unique_ptr<SEGS::SceneGraph> m_scene_graph;
    QSet<QByteArray> missing_geosets;
    m_scene_graph.reset(SEGS::loadSceneGraphNoNesting(&se_wrap, fs_path.c_str(),missing_geosets));
    if (!m_scene_graph)
    {
        PLUG_FAIL_V_MSG(ERR_FILE_CORRUPT, "Failed to load the original scene graph : path problems?");
    }
    for(auto iter=m_scene_graph->m_requests.begin(); iter!= m_scene_graph->m_requests.end(); ++iter)
    {
        QString packed_in    = iter.key().base_file + "/" + QFileInfo(packed_in).fileName() + ".bin";
        String needs_library = String("res://coh_data/geobin/")+qPrintable(packed_in);

        qDebug() << packed_in;

        if(!FileAccess::exists(needs_library))
            needs_library = qPrintable(getFilepathCaseInsensitive(se_wrap,needs_library.c_str()));

        if (!FileAccess::exists(needs_library))
        {
            r_missing_deps.emplace_back(needs_library);
        }
        else
        {
            for (SEGS::NodeLoadTarget& load_tgt : *iter)
            {
                LibraryEntryInstance * inst=memnew(LibraryEntryInstance);
                inst->set_library_path(needs_library);
                    inst->set_entry(iter.key().node_name.data());

                SEGS::SceneNode *imported_node;
                if (load_tgt.node != nullptr)
                { // internal node
                    inst->set_name(String(String::CtorSprintf(),"%p_%s",inst,iter.key().node_name.data()));
                    imported_node           = new SEGS::SceneNode(load_tgt.node->m_nest_level + 1);
                    load_tgt.node->m_children[load_tgt.child_idx].node = imported_node;
                }
                else
                {
                    assert(nullptr==m_scene_graph->roots[load_tgt.child_idx]->node);
                    imported_node = new SEGS::SceneNode(0);
                    m_scene_graph->roots[load_tgt.child_idx]->node = imported_node;
                }
                imported_node->m_engine_node = inst;
            }
        }
    }
    if(!r_missing_deps.empty())
        return ERR_FILE_MISSING_DEPENDENCIES;
    Ref<SceneLibrary> part_lib(build_scene_library(*m_scene_graph,r_missing_deps,p_source_file));
    for (auto dep : r_missing_deps)
    {
        getCoreInterface()->reportError("Missing dependency:"+dep,"",FUNCTION_STR, __FILE__, __LINE__);
    }
    for (auto dep : missing_geosets)
    {
        getCoreInterface()->reportError(String("Missing geoset:")+dep.data(),"",FUNCTION_STR, __FILE__, __LINE__);
    }

    //TODO: for now we allow missing anything.
    r_missing_deps.clear();
    String save_path = String(PathUtils::get_basename(p_source_file)) + ".tscn";

    if (!m_scene_graph->roots.empty())
    {
        Ref<PackedScene> secondary(make_ref_counted<PackedScene>());
        Node3D* root = memnew(Node3D);
        root->set_name(PathUtils::get_basename(p_source_file));
        for(const auto & r : m_scene_graph->roots)
        {
            LibraryEntryInstance* entry;
            if (r->node->m_engine_node)
            {
                entry = object_cast<LibraryEntryInstance>((Object *)r->node->m_engine_node);
            }
            else
            {
                entry = memnew(LibraryEntryInstance);
                entry->set_library_path(String(p_source_file));
                entry->set_entry(r->node->m_name.data());
                entry->set_name(qPrintable(r->node->m_name));

            }
            root->add_child(entry);
            entry->set_owner(root);
            static_cast<Node3D*>(entry)->set_transform(fromGLM(r->mat));

        }
        secondary->pack(root);

        gResourceManager().save(save_path, secondary);
        if (r_gen_files) {
            r_gen_files->push_back(save_path);
        }
    }
    Error res = gResourceManager().save(String(p_save_path) + ".scenelib", part_lib);
//    auto v(part_lib->get_item_list());
//    for(int i : v){
//        gResourceManager().save(String(p_save_path) + StringUtils::num(i) + ".tscn", part_lib->get_item_scene(i));
//    }
    return res;
}
