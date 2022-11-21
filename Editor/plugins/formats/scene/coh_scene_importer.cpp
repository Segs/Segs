/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2022 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "coh_scene_importer.h"


#include "Common/GameData/scenegraph_serializers.h"
#include "Common/Runtime/Model.h"
#include "GameData/trick_definitions.h"
#include "Prefab.h"
#include "RuntimeData.h"
#include "SceneGraph.h"
#include <core/class_db.h>
#include <core/io/image_loader.h>
#include <core/io/resource_loader.h>
#include <core/os/dir_access.h>
#include <core/os/mutex.h>
#include <core/project_settings.h>
#include <core/reference.h>
#include <core/resource/resource_manager.h>
#include <core/service_interfaces/CoreInterface.h>
#include <core/string_utils.h>
#include <scene/3d/instantiation.h>
#include <scene/3d/light_3d.h>
#include <scene/3d/mesh_instance_3d.h>
#include <scene/3d/node_3d.h>

#include <scene/resources/packed_scene.h>
#include <scene/resources/primitive_meshes.h>
#include <scene/resources/scene_library.h>
#include <scene/resources/texture.h>

#include "glm/gtx/matrix_decompose.hpp"

#include <QDebug>
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
    ~FileIOWrap() override {}
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
static Node *convertFromRoot(SceneGraphInfo &sg, Node *root, Node *parent, SEGS::SceneNode *n);

static void convertLightComponent(SEGS::SceneNode *n, Node3D *res)
{
    //OmniLight3D *light_node = memnew(OmniLight3D);
    Node3D *light_placeholder = memnew(Node3D);
    //if (n->m_light->is_negative)
    //{
    //    light_node->set_negative(true);
    //}
    // since light object is disabled by default, we don't want to disable ourselves,
    // since we have light probes
    // Light objects are put into seperate layer, to allow fast sphere collider lookups.
    //light_node->set_layer_mask();
    //lobj.layer = LayerMask.NameToLayer("OmniLights");
    light_placeholder->set_name("OmniLight");
    res->add_child(light_placeholder);
    light_placeholder->set_owner(res->get_owner());

    //light_node->set_color(Color(fromGLM(n->m_light->color)));
    //light_node->set_param(Light3D::PARAM_RANGE,n->m_light->range);
    //light_node->set_bake_mode(Light3D::BAKE_INDIRECT);

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
namespace
{
#pragma pack(push, 1)
    struct TexFileHdr
    {
        int     header_size;
        int     file_size;
        int     wdth;
        int     hght;
        int     flags;
        int     fade[2];
        uint8_t alpha;
        char    magic[3];
    };
#pragma pack(pop)
    QSet<QString>                  s_missing_textures;
    std::unordered_map<uint32_t, Ref<Texture>> g_converted_textures;
}

/*std::vector<SEGS::HTexture> getModelTextures(std::vector<QByteArray> &names)
{
    uint32_t name_count = std::max<uint32_t>(1,names.size());
    std::vector<SEGS::HTexture> res;
    res.reserve(name_count);
    SEGS::HTexture white_tex = tryLoadTexture("white.tga");

    for(size_t tex_idx=0; tex_idx < names.size(); ++tex_idx )
    {
        QFileInfo fi(names[tex_idx]);
        QByteArray baseName = fi.completeBaseName().toUtf8();
        if(baseName!=names[tex_idx])
        {
            if(fi.fileName() == names[tex_idx])
                names[tex_idx] = baseName;
            else
                names[tex_idx] = (fi.path()+"/"+baseName).toUtf8();
        }
        if ( names[tex_idx].toUpper().contains("PORTAL") )
            res.emplace_back(tryLoadTexture("invisible.tga"));
        else
            res.emplace_back(tryLoadTexture(names[tex_idx]));
        // replace missing texture with white
        // TODO: make missing textures much more visible ( high contrast + text ? )
        if ( g_converted_textures.end()==g_converted_textures.find(res[tex_idx].idx) )
        {
            res[tex_idx] = white_tex;
        }
    }
    if (names.empty())
        res.emplace_back(white_tex);
    return res;
}*/

static void convertModel(SEGS::Model*mdl, Node3D *res, HashSet<String> &missing_models)
{
    ModelModifiers *model_trick = mdl->trck_node;
    if (model_trick)
    {
        if (model_trick->isFlag(NoDraw))
        {
            // qDebug() << mdl->name << "Set as no draw";
            return;
        }
        if (model_trick->isFlag(EditorVisible))
        {
            // qDebug() << mdl->name << "Set as editor model";
            return;
        }
        if (model_trick && model_trick->isFlag(CastShadow))
        {
            // qDebug() << "Not converting shadow models"<<mdl->name;
            return;
        }
        if (model_trick && model_trick->isFlag(ParticleSys))
        {
            return;
        }
    }
    String meshlib_path = PathUtils::get_base_dir(mdl->geoset->geopath.data());
    String geoset_file(PathUtils::get_basename(StringView(mdl->geoset->geopath.data())));
    String mesh_name = qPrintable(mdl->name);


    size_t obj_lib_idx = meshlib_path.find("object_library");
    if (obj_lib_idx != String::npos)
        meshlib_path = "coh_data/" + meshlib_path.substr(obj_lib_idx);
    SE_FSWrapper wrap;
    MeshInstance3D *mi = memnew(MeshInstance3D);
    res->add_child(mi);

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
    if (!mdl->geoset->data_loaded)
{
        String basepath = ProjectSettings::get_singleton()->get_resource_path() + "/coh_data/";
        auto res = wrap.open(QString::fromUtf8(basepath.c_str()) + "/" + mdl->geoset->geopath.data(), true, false);

        SEGS::geosetLoadData(res,mdl->geoset);
        if (!mdl->geoset->subs.empty())
        {
            //std::vector<SEGS::HTexture> model_textures = getModelTextures(mdl->geoset->tex_names);
            for (SEGS::Model* model : mdl->geoset->subs)
            {
                //s_coh_model_to_engine_model[model] = modelCreateObjectFromModel(ctx, model, model_textures);
        }
    }

//        if (mdl.geoset.subs.Count != 0)
//            mdl.geoset.createEngineModelsFromPrefabSet();
        if (res)
        {
            res->close();
            delete res;
}
    }
//    if (mf.sharedMesh == null)
{
        MeshInstance3D *res_static=nullptr;
        SEGS::RuntimeData &rd(getRuntimeData());
//        if (!rd.s_coh_model_to_engine_model.TryGetValue(mdl, out res_static))
//            return;
        if (res_static == nullptr)
            return;
//        SEGSRuntime.Tools.EnsureDirectoryExists(mesh_path);
//        AssetDatabase.CreateAsset(res_static.m_mesh, model_path);
//        AssetDatabase.SaveAssets();
//        mf.sharedMesh = AssetDatabase.LoadAssetAtPath<Mesh>(model_path);
    }

//    convertMaterials(ren, mdl, res);
    qDebug("Not converting models yet");
}
void createMat3YPR(Basis &mat, Vector3 pyr)
    {
    float sinP = Math::sin(pyr[0]);
    float cosP = Math::cos(pyr[0]);
    float sinY = Math::sin(pyr[1]);
    float cosY = Math::cos(pyr[1]);
    float sinR = Math::sin(pyr[2]);
    float cosR = Math::cos(pyr[2]);

    float temp = sinY * sinP;
    mat[0][0]  = cosY * cosR + temp * sinR;
    mat[0][1]  = cosY * sinR - temp * cosR;
    mat[0][2]  = sinY * cosP;

    mat[1][0] = -cosP * sinR;
    mat[1][1] = cosP * cosR;
    mat[1][2] = sinP;

    temp      = -cosY * sinP;
    mat[2][0] = -sinY * cosR - temp * sinR;
    mat[2][1] = -sinY * sinR + temp * cosR;
    mat[2][2] = cosY * cosP;
    }

static Transform fromCOH(glm::vec3 m_pyr, glm::vec3 m_translation)
    {
    Transform tr;
    Basis     tgt;
    createMat3YPR(tgt, Vector3(m_pyr.x, m_pyr.y, m_pyr.z));
    Basis tgt2;
    tgt2.set_euler_yxz(Vector3(-m_pyr.x, m_pyr.y, -m_pyr.z));
    assert(tgt.is_equal_approx(tgt2));

    tr.translate(m_translation.x, m_translation.y, m_translation.z);
    tr.set_basis(tgt);
    return tr;
    }
static void createNodeInstances(SEGS::SceneGraph &m_scene_graph, SceneGraphInfo &sg)
{
    // at this point we leave the hierarchy to original scene graph, for every scene graph node we create
    // a SE node either as a LibraryInstance or direct node pointer

    for (auto *node : m_scene_graph.all_converted_defs)
{
        if (node->m_engine_node) // already converted ? external reference or similar
    {
            continue;
    }
        Node3D *res = memnew(Node3D);
        res->set_name(qPrintable(node->m_name));
        // Convert all scene node components

        convertComponents(node, res);
        // convert model
        if (node->m_model != nullptr)
    {
            convertModel(node->m_model, res, sg.missing_imported_geosets);
    }
        node->m_engine_node = res;
}

}

static void resolve_children(SEGS::SceneNode *r, StringView p_source_file)
{
    Node *n = static_cast<Node *>(r->m_engine_node);
    assert(n);
    for (const SEGS::SceneNodeChildTransform &child : r->m_children)
{
        Node3D *child_node = static_cast<Node3D *>(child.node->m_engine_node);

        if (child.node->m_use_count > 1) // we need to use scene library instancing
{
            // this child links to a node with multiple uses : convert to LibraryEntryInstance
            auto *existing = object_cast<LibraryEntryInstance>(child_node);
            if (existing)
            { // external scene library reference
                child_node = (Node3D *)existing->duplicate();
}

            else
{
                existing = memnew(LibraryEntryInstance);
                existing->set_library_path(String(p_source_file));
                existing->set_entry(child.node->m_name.data());
                existing->set_name(qPrintable(child.node->m_name));
                child_node = existing;
}

}

        n->add_child(child_node);
        Transform tr(fromCOH(child.m_pyr, child.m_translation));
        child_node->set_transform(tr);
    }
}
static void set_owner_deep(Node *owner, Node *start)
{

    if (start != owner)
    {
        start->set_owner(owner);
}

    for (Node *n : start->children())
{
        set_owner_deep(owner, n);
}
}
// visit all 'root' nodes and set ownership on all their children
static void resolve_ownership(SEGS::SceneGraph &m_scene_graph)
{
    // visit all 'roots'
    // root is : all nodes with either 0 uses ( very likely exported ones ), or >1 uses ( internal prefabs )
    for (const auto &r : m_scene_graph.all_converted_defs)
    {
        if (r->m_use_count == 1 || r->m_nest_level!=0)
            continue;
        Node *owner = static_cast<Node *>(r->m_engine_node);
        set_owner_deep(owner, owner);
}
}
static void select_highest_quality_lods(SEGS::SceneNode *r)
{
    Node *n = static_cast<Node *>(r->m_engine_node);
    assert(n);
    for (const SEGS::SceneNodeChildTransform &child : r->m_children)
    {
        if (child.node->lod_near>0)
        {
            Node3D *n = (Node3D *)child.node->m_engine_node;
            n->hide();
        }
    }
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
    createNodeInstances(m_scene_graph, sg);
    // at this point all scene nodes have associated engine nodes.
    // now we rebuild the hierarchy by visiting all nodes that have children and resolving those as local or lib nodes
    for (SEGS::SceneNode *r : m_scene_graph.all_converted_defs)
    {
        resolve_children(r, p_source_file);
    }

    resolve_ownership(m_scene_graph);
    for (SEGS::SceneNode *r : m_scene_graph.all_converted_defs)
    {
        select_highest_quality_lods(r);
    }

    // now we can create prefabs from all roots.
    for (const auto &r : m_scene_graph.all_converted_defs)
    {
        if (r->m_use_count == 1)
            continue;
        Node *top_level = static_cast<Node *>(r->m_engine_node);

        if (sg.lib->find_item_by_name(r->m_name.data()) == LibraryItemHandle(-1))
        {
            SceneLibrary::Item dat;
            dat.scene = make_ref_counted<PackedScene>();
            dat.scene->pack(top_level);
            memdelete(top_level);
            dat.name = r->m_name.data();
            auto inst = memnew(LibraryEntryInstance);
            inst->set_library_path(String(p_source_file));
            inst->set_entry(r->m_name.data());
            inst->set_name(qPrintable(r->m_name));
            r->m_engine_node = inst;
            sg.lib->add_item(eastl::move(dat));
        }
    }
    missing_resources.insert(missing_resources.end(), sg.missing_imported_geosets.begin(),
                             sg.missing_imported_geosets.end());
    return eastl::move(sg.lib);
}
#if 0
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
#endif

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

StringName CoHSceneLibrary::get_preset_name(int p_idx) const {

    return StringName();
}

bool CoHSceneLibrary::get_option_visibility(const StringName &p_option,
    const HashMap<StringName, Variant> &p_options) const
{
    return true;
}

Error CoHSceneLibrary::import(StringView p_source_file, StringView p_save_path,
                              const HashMap<StringName, Variant> &p_options, Vector<String> &r_missing_deps,
                              Vector<String> *r_platform_variants, Vector<String> *r_gen_files, Variant *r_metadata)
{
    using namespace StringUtils;

    SE_FSWrapper se_wrap;

    // Check if the selected file is correctly located in the hierarchy, just to make sure.
    SceneGraphInfo sg_info;
    auto idx = StringUtils::rfind(p_source_file, "geobin");
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
        SEGS::preloadTextureNames(&se_wrap, basepath.c_str());
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
        const SEGS::NodeLoadRequest &reqest(iter.key());
        QString packed_in = reqest.base_file + "/" + QFileInfo(reqest.base_file).fileName() + ".bin";
        String needs_library = String("res://coh_data/geobin/")+qPrintable(packed_in);

        //qDebug() << packed_in;

        if(!FileAccess::exists(needs_library))
        {
            needs_library = qPrintable(getFilepathCaseInsensitive(se_wrap,needs_library.c_str()));
        }

        if (!FileAccess::exists(needs_library))
        {
            r_missing_deps.emplace_back(needs_library);
            continue;
        }
            for (SEGS::NodeLoadTarget& load_tgt : *iter)
            {
                LibraryEntryInstance * inst=memnew(LibraryEntryInstance);
                inst->set_library_path(needs_library);
                    inst->set_entry(iter.key().node_name.data());

                SEGS::SceneNode *imported_node;
                if (load_tgt.node != nullptr)
            {
                // internal node
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
            // this external node is used in multiple places
            imported_node->m_use_count   = (*iter).size();
                imported_node->m_engine_node = inst;
            }
        }
    if(!r_missing_deps.empty())
    {
        return ERR_FILE_MISSING_DEPENDENCIES;
    }
    Ref<SceneLibrary> part_lib(build_scene_library(*m_scene_graph,r_missing_deps,p_source_file));
    //TODO: add a list of SceneLibrary<->SceneLibrary dependencies here


    for (const String &dep : r_missing_deps)
    {
        getCoreInterface()->reportError("Missing dependency:"+dep,"",FUNCTION_STR, __FILE__, __LINE__);
    }
    for (const auto &dep : missing_geosets)
    {
        getCoreInterface()->reportError(String("Missing geoset:")+dep.data(),"",FUNCTION_STR, __FILE__, __LINE__);
    }

    //TODO: for now we allow missing anything.
    r_missing_deps.clear();
    String save_path = String(PathUtils::get_basename(p_source_file)) + ".tscn";

    if (!m_scene_graph->roots.empty())
    {
        // an actual scene file needs to be produced, since we have roots
        Node3D* root = memnew(Node3D);
        root->set_name(PathUtils::get_basename(p_source_file));
        for(const auto & r : m_scene_graph->roots)
        {
            LibraryEntryInstance* entry;
            if (r->node->m_engine_node)
            {
                auto vv = (Node *)r->node->m_engine_node;
                entry = object_cast<LibraryEntryInstance>((Node *)r->node->m_engine_node);
            }
            else
            {
                entry = memnew(LibraryEntryInstance);
                entry->set_library_path(String(p_source_file));
                entry->set_entry(r->node->m_name.data());
                entry->set_name(qPrintable(r->node->m_name));

                r->node->m_engine_node = entry;
            }
            root->add_child(entry);
            entry->set_owner(root);
            entry->set_transform(fromCOH(r->rot, r->pos));

        }
        Ref<PackedScene> secondary(make_ref_counted<PackedScene>());
        secondary->pack(root);
        memdelete(root);

        gResourceManager().save(save_path, secondary);
        if (r_gen_files) {
            r_gen_files->push_back(save_path);
        }
    }
    Error res = gResourceManager().save(String(p_save_path) + ".scenelib", part_lib);
    //gResourceManager().save(String(p_save_path) + ".tres", part_lib);
//    auto v(part_lib->get_item_list());
//    for(int i : v){
//        gResourceManager().save(String(p_save_path) + StringUtils::num(i) + ".tscn", part_lib->get_item_scene(i));
//    }
    return res;
}
