#include "SceneGraph.h"

#include "CompiletimeHash.h"
#include "RuntimeData.h"
#include "Model.h"
#include "Prefab.h"
#include "Sound.h"

#include "GameData/scenegraph_definitions.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "GameData/DataStorage.h"
#include "GameData/CoHMath.h"
#include "Common/Runtime/Prefab.h"
#include "Logging.h"

#include "glm/mat3x3.hpp"
#include "glm/gtx/quaternion.hpp"
#include <QDir>
#include <cmath>

using namespace SEGS;

// Node name re-mapping support

namespace
{

struct TexBlockInfo
{
    uint32_t size1;
    uint32_t texname_blocksize;
    uint32_t bone_names_size;
    uint32_t tex_binds_size;
};

struct GeosetHeader32
{
    char name[124];
    int  parent_idx;
    int  unkn1;
    int  subs_idx;
    int  num_subs;
};

struct PackInfo
{
    int      compressed_size;
    uint32_t uncomp_size;
    int      compressed_data_off;
};

static_assert(sizeof(PackInfo) == 12, "sizeof(PackInfo)==12");
struct Model32
{
    int             flg1;
    float           radius;
    int             vbo;
    uint32_t        num_textures;
    int16_t         id;
    char            blend_mode;
    char            loadstate;
    int             boneinfo;
    int             trck_node;
    uint32_t        vertex_count;
    uint32_t        model_tri_count;
    int             texture_bind_offsets;
    int             unpacked_1;
    glm::vec3       grid_pos;
    float           grid_size;
    float           grid_invsize;
    float           grid_tag;
    int             grid_numbits;
    int             ctris;
    int             triangle_tags;
    int             bone_name_offset;
    int             num_altpivots;
    int             extra;
    glm::vec3       m_scale;
    glm::vec3       m_min;
    glm::vec3       m_max;
    int             geoset_list_idx;
    PackInfo        pack_data[7];
};

bool groupInLibSub(const QString &name)
{
    if(name.contains('/'))
        return !name.startsWith("maps");

    return !name.startsWith("grp");
}

uint32_t is_flag_set(uint32_t bf,GroupFlags flg)
{
    return (bf&flg)!=0;
}

} // end of anonymous namespace

namespace SEGS
{

SceneNode * getNodeByName(const SceneGraph &graph,const QString &name)
{
    QString filename;
    int idx = name.lastIndexOf('/');
    if( idx==-1 )
        filename = name;
    else
        filename = name.mid(idx+1);

    return graph.name_to_node.value(filename.toLower(), nullptr);
}

} // end of SEGS namespace

QString  groupMakeName(const QString &base,LoadingContext &ctx)
{
    QString buf;
    do
        buf = base + QString::number(++ctx.last_node_id);
    while (getNodeByName(*ctx.m_target,buf));

    return buf;
}

// Create new names for any 'numbered' scene nodes
QString groupRename(LoadingContext &ctx, const QString &oldname, bool is_def)
{
    QString str;
    str = oldname.contains('/') ? oldname : ctx.m_renamer.basename+'/'+oldname;
    if( groupInLibSub(str) )
        return str;
    if( !is_def && !str.contains("/grp",Qt::CaseInsensitive) && !str.contains("/map",Qt::CaseInsensitive) )
        return str;

    QString querystring = str.toLower();
    auto str_iter = ctx.m_renamer.new_names.find(querystring);

    if( str_iter!=ctx.m_renamer.new_names.end() )
        return *str_iter;

    QString prefix = str;
    int gidx = prefix.indexOf("/grp",0,Qt::CaseInsensitive);
    if( gidx!=-1 )
    {
        prefix = prefix.mid(gidx+4); // skip /grp
        prefix = prefix.mid(0,prefix.indexOf(QRegExp("[^A-Za-z]"))); // collect chars to first non-letter
    }
    else
    {
        if( prefix.contains("/map",Qt::CaseInsensitive) )
            prefix = "maps/grp";
        else
        {
            qCDebug(logSceneGraph) << "bad def name:" << prefix;
            prefix = "baddef";
        }
    }
    QString tgt = groupMakeName(prefix,ctx);
    ctx.m_renamer.new_names[querystring] = tgt;
    return tgt;
}

QString buildBaseName(const QString& path)
{
    QStringList z = path.split(QDir::separator());

    if(z.size()>1)
        z.pop_back(); // remove file name

    if(z.front()=="object_library")
        z.pop_front();
    else if(z.contains("maps",Qt::CaseInsensitive))
    {
        while(0!=z.front().compare("maps",Qt::CaseInsensitive))
            z.pop_front();
    }

    return z.join('/');
}

QString mapNameToPath(const QString &name,LoadingContext &ctx)
{
    int start_idx = name.indexOf("object_library",Qt::CaseInsensitive);
    if( -1==start_idx )
        start_idx = name.indexOf("maps",Qt::CaseInsensitive);

    QString buf = ctx.m_base_path+"geobin/" + name.midRef(start_idx);

    const int last_dot = buf.lastIndexOf('.');
    if(-1==last_dot)
        buf+=".bin";
    else if(!buf.contains(".crl"))
        buf.replace(last_dot,buf.size()-last_dot,".bin");

    return buf;
}

RootNode *newRef(SceneGraph &scene)
{
    size_t idx;
    for(idx=0; idx<scene.refs.size(); ++idx)
        if(!scene.refs[idx] || !scene.refs[idx]->node)
            break;

    if(idx>=scene.refs.size())
    {
        idx = scene.refs.size();
        scene.refs.emplace_back();
    }

    scene.refs[idx] = new RootNode;
    scene.refs[idx]->index_in_roots_array = idx;
    return scene.refs[idx];
}

void addRoot(const SceneRootNode_Data &refload, LoadingContext &ctx, PrefabStore &store)
{
    QString newname = groupRename(ctx, refload.name, false);
    auto *def = getNodeByName(*ctx.m_target,newname);
    if(!def)
    {
        if(store.loadNamedPrefab(newname,ctx))
        {
            def = getNodeByName(*ctx.m_target,newname);
        }
    }
    if(!def)
    {
        qCritical() << "Missing reference:" << newname;
        return;
    }
    auto ref = newRef(*ctx.m_target);
    ref->node = def;
    transformFromYPRandTranslation(ref->mat,{refload.rot.x,refload.rot.y,refload.rot.z},refload.pos);
}

SceneNode *newDef(SceneGraph &scene)
{
    SceneNode *res = new SceneNode;
    res->m_index_in_scenegraph = scene.all_converted_defs.size();
    scene.all_converted_defs.emplace_back(res);
    res->in_use = true;
    return res;
}

void setNodeNameAndPath(SceneGraph &scene,SceneNode *node, QString obj_path)
{
    QString result;
    size_t strlenobjec = strlen("object_library");
    if( obj_path.startsWith("object_library", Qt::CaseInsensitive) )
        obj_path.remove(0, strlenobjec + 1);
    if( groupInLibSub(obj_path) )
        result = "object_library/";

    result += obj_path;
    int last_separator = result.lastIndexOf('/');
    QStringRef key = result.midRef(last_separator + 1);
    QString lowkey = key.toString().toLower();

    auto iter = scene.name_to_node.find(lowkey);
    if(iter==scene.name_to_node.end())
        scene.name_to_node[lowkey] = node;

    node->m_name = key.toString();
    node->m_dir.clear();

    if( key.position() != 0 )
        node->m_dir = result.mid(0,key.position()-1);
}

void addChildNodes(const SceneGraphNode_Data &inp_data, SceneNode *node, LoadingContext &ctx, PrefabStore &store)
{
    if( inp_data.p_Grp.empty() )
        return;

    node->m_children.reserve(inp_data.p_Grp.size());
    for(const GroupLoc_Data & dat : inp_data.p_Grp)
    {
        const QString new_name = groupRename(ctx, dat.name, false);
        SceneNodeChildTransform child;
        child.node = getNodeByName(*ctx.m_target, new_name);
        if( !child.node )
        {
            bool loaded = store.loadNamedPrefab(new_name, ctx);
            if(!loaded)
                qCritical() << "Cannot load named prefab" << new_name << "result is" << loaded;
            child.node = getNodeByName(*ctx.m_target, new_name);
        }
        // construct from euler angles
        glm::quat qPitch = glm::angleAxis(dat.rot.x, glm::vec3(-1, 0, 0));
        glm::quat qYaw =  glm::angleAxis(dat.rot.y, glm::vec3(0, 1, 0));
        glm::quat qRoll = glm::angleAxis(dat.rot.z, glm::vec3(0, 0, 1));
        glm::quat rotQuat = qYaw * qPitch * qRoll;
        child.m_pyr = dat.rot;
        child.m_matrix2 = glm::mat3(rotQuat);
        child.m_translation = dat.pos;
        if( child.node )
            node->m_children.emplace_back(child);
        else
            qCritical() << "Node" << node->m_name << "\ncan't find member" << dat.name;
    }
}
void postprocessLOD(const std::vector<DefLod_Data> &lods, SceneNode *node)
{
    if(lods.empty())
        return;

    const DefLod_Data &lod_data(lods.front());
    node->lod_scale = lod_data.Scale;

    if( node->lod_fromtrick )
        return;

    node->lod_far       = lod_data.Far;
    node->lod_far_fade  = lod_data.FarFade;
    node->lod_near      = lod_data.Near;
    node->lod_near_fade = lod_data.NearFade;
}

void postprocessLight(const std::vector<DefOmni_Data> & light_data, SceneNode *node)
{
    if( light_data.empty() )
        return;
    const DefOmni_Data &omnid(light_data.front());
    node->m_light = std::make_unique<LightProperties>(LightProperties{
        RGBA(omnid.omniColor).toFloats(),
        omnid.Size,
        omnid.isNegative
    });
}

bool nodeCalculateBounds(SceneNode *group)
{
    float geometry_radius=0.0f;
    float maxrad=0.0f;
    Model *model;
    AxisAlignedBoundingBox bbox;
    bool set = false;
    if( !group )
        return false;

    if( group->m_model )
    {
        model = group->m_model;
        bbox.merge(model->box);

        geometry_radius = glm::length(bbox.size()) * 0.5f;
        set = true;
    }

    for ( SceneNodeChildTransform & child : group->m_children )
    {
        glm::vec3 dst(child.node->m_center * child.m_matrix2 + child.m_translation);
        glm::vec3 v_radius(child.node->radius,child.node->radius,child.node->radius);
        bbox.merge(dst+v_radius);
        bbox.merge(dst-v_radius);
        set = true;
    }

    if( !set )
        bbox.clear();

    group->radius = glm::length(bbox.size()) * 0.5f;
    group->m_bbox = bbox;
    group->m_center = bbox.center(); // center
    for ( SceneNodeChildTransform & child : group->m_children )
    {
        glm::vec3 toChildCenter = (child.m_matrix2 * child.node->m_center + child.m_translation) - group->m_center;
        float r = glm::length(toChildCenter) + child.node->radius;
        maxrad = std::max(maxrad,r);
    }

    if( maxrad != 0.0f )
        group->radius = maxrad;

    group->radius = std::max(geometry_radius,group->radius);
    return group->radius == 0.0f && !group->m_children.empty();
}

void  nodeSetVisBounds(SceneNode *group)
{
    //TODO: fix this
    glm::vec3 dv;
    float maxrad = 0.0;
    float maxvis = 0.0;

    if( !group )
        return;
    if( group->lod_scale == 0.0f )
        group->lod_scale = 1.0f;

    if( group->m_model )
    {
        Model *model = group->m_model;
        dv = model->box.size();
        maxrad = glm::length(dv) * 0.5f + group->shadow_dist;
        if( group->lod_far == 0.0f )
        {
            group->lod_far = (maxrad + 10.0f) * 10.0f;
            group->lod_far_fade = group->lod_far * 0.25f;
            // lod_far is autogenned here
        }
        maxvis = group->lod_far + group->lod_far_fade;
    }

    for (SceneNodeChildTransform &entr : group->m_children)
    {
        dv = entr.m_matrix2 * entr.node->m_center + entr.m_translation;
        dv -= group->m_center;
        maxrad = std::max(maxrad,glm::length(dv) + entr.node->radius + entr.node->shadow_dist);
        maxvis = std::max(maxvis,glm::length(dv) + entr.node->vis_dist * entr.node->lod_scale);
    }

    if( group->shadow_dist == 0.0f )
        group->shadow_dist = maxrad - group->radius;

    group->vis_dist = maxvis;
}
void postprocessSound(const std::vector<DefSound_Data> &data,SceneNode *node)
{
    if(data.empty())
        return;
    //TODO: consider multiple sounds per-node to reduce node spam ?
    assert(data.size()==1);
    const DefSound_Data &snd(data.front());
    HSound handle = SoundStorage::instance().create();
    handle->name = snd.name;
    handle->radius = snd.sndRadius;
    handle->ramp_feet = snd.snd_ramp_feet;
    handle->flags = snd.sndFlags;
    node->sound_info = handle;
}

void postprocessEditorBeacon(const std::vector<DefBeacon_Data> &data, SceneNode * /*node*/)
{
    if( data.empty())
        return;
    // mostly markers like TrafficBeacon/CombatBeacon/BasicBeacon
    const DefBeacon_Data &bcn(data.front());
//TODO: consider if we want to allow the use of editor beacons ?
//        HBeacon b = BeaconStorage::instance().create();
//        b->name = bcn.name;
//        b->radius = bcn.amplitude;
//        node->m_editor_beacon=b;
}

void postprocessFog(const std::vector<DefFog_Data> &data, SceneNode * /*node*/)
{
    //TODO: only 1 fog value is used here, either change the source structure or consider how multi-fog would work ?
    if( data.empty() )
        return;

    const DefFog_Data &fog_data(data.front());
    //TODO: MapViewer does not handle this info yet
    // HFog f = FogInfoStorage::instance().create()
    // f->color_1 = fog_data.fogClr1;
    // f->color_2 = fog_data.fogClr2;
    // f->radius = fog_data.fogZ;
    // f->near = fog_data.fogX;
    // f->far = fog_data.fogY;
}

void postprocessAmbient(const std::vector<DefAmbient_Data> &data, SceneNode * /*a2*/)
{
    //TODO: only one value is used here, either change the source structure or consider how multi-ambient would work ?
    if( data.empty() )
        return;

    const DefAmbient_Data &light_data(data.front());
    //NOTE: original engine used the same value for first fog color and ambient light!
    //TODO: MapViewer does not handle this info yet
    // HAmbientLight l = AmbientLightStorage::instance().create()
    // l->color = light_data.clr;
}

void postprocessTintColor(const std::vector<TintColor_Data> &data, SceneNode * /*node*/)
{
    //TODO: only 1 tint is used here, either change the source structure or consider how multi-tint would work ?
    if( data.empty() )
        return;
    const TintColor_Data &tint_data(data.front());
    //TODO: MapViewer does not handle this case
    //Nodes with a tint set could use that value, if proper ModelModifiers flag was set.
    //ColorOnly models would use first tint color
    //DistAlpha would use alpha from first tint color
    //SetColor would set the blend colors to tint values
}

void postprocessTextureReplacers(const std::vector<ReplaceTex_Data> &data, SceneNode * /*node*/)
{
    //WARNING: This needs pretty urgent attention, MapViewer does not handle this at all
    // and it's a pretty important piece of the puzzle.
    for (const ReplaceTex_Data &tex_repl : data )
    {
        qCDebug(logSceneGraph) << "Texture to Replace:" << tex_repl.repl_with;
        // HInstanceMod tr = InstanceModStorage::instance().create();
        // tr->addTextureReplacement(tex_repl.texUnit,tex_repl.repl_with);
    }
}

void postprocessNodeFlags(const SceneGraphNode_Data & node_data, SceneNode * node)
{
    if( node_data.flags & SceneGraphNode_Data::Ungroupable )
    {
//        node->is_ungroupable = 1; // only useful for editing
    }

    if( node_data.flags & SceneGraphNode_Data::FadeNode )
        node->is_LOD_fade_node = 1;
}

void  groupApplyModifiers(SceneNode *node)
{
    RuntimeData &rd(getRuntimeData());
    
    Model *model = node->m_model;
    if( !model )
        return;
    GeometryModifiers *mods = findGeomModifier(*rd.m_modifiers,model->name, node->m_dir);
    if( !mods )
        return;

    if( mods->LodNear != 0.0f )
        node->lod_near = mods->LodNear;
    if( mods->LodFar != 0.0f )
        node->lod_far = mods->LodFar;
    if( mods->LodNearFade != 0.0f )
        node->lod_near_fade = mods->LodNearFade;
    if( mods->LodFarFade != 0.0f )
        node->lod_far_fade = mods->LodFarFade;
    if( mods->LodScale != 0.0f )
        node->lod_scale = mods->LodScale;

    uint32_t v1 = mods->GroupFlags;
    node->shadow_dist    = mods->ShadowDist;
    node->parent_fade    = is_flag_set(v1, ParentFade);
    node->region_marker  = is_flag_set(v1, RegionMarker);
    node->volume_trigger = is_flag_set(v1, VolumeTrigger);
    node->water_volume   = is_flag_set(v1, WaterVolume);
    node->lava_volume    = is_flag_set(v1, LavaVolume);
    node->sewer_volume   = is_flag_set(v1, SewerWaterVolume);
    node->door_volume    = is_flag_set(v1, DoorVolume);
    node->key_light      = is_flag_set(v1, KeyLight);
    node->tray           = is_flag_set(v1, VisTray) | is_flag_set(v1, VisOutside);

    if(mods->LodNear != 0.0f || mods->LodFar != 0.0f || mods->LodNearFade != 0.0f || mods->LodFarFade != 0.0f || mods->LodScale != 0.0f)
        node->lod_fromtrick = 1;
    if( mods->node._TrickFlags & NoColl )
        ; //TODO: disable collisions for this node
    if( mods->node._TrickFlags & SelectOnly )
        ; // set the model's triangles as only selectable ?? ( selection mesh ? )
    if( mods->node._TrickFlags & NotSelectable )
        ; // 
}
bool addNode(const SceneGraphNode_Data &defload, LoadingContext &ctx,PrefabStore &prefabs)
{
    if(defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;

    QString obj_path = groupRename(ctx, defload.name, true);
    SceneNode * node = getNodeByName(*ctx.m_target,obj_path);
    if(!node)
    {
        node = newDef(*ctx.m_target);
        if(!defload.p_Property.empty())
            node->m_properties = new std::vector<GroupProperty_Data> (defload.p_Property);
    }

    if( !defload.p_Obj.isEmpty() )
    {
        node->m_model = prefabs.groupModelFind(defload.p_Obj,ctx);
        if( !node->m_model )
            qCritical() << "Cannot find root geometry in" << defload.p_Obj;

        groupApplyModifiers(node);
    }
    setNodeNameAndPath(*ctx.m_target,node,obj_path);
    addChildNodes(defload,node,ctx,prefabs);

    if( node->m_children.empty() && !node->m_model )
    {
        qCDebug(logSceneGraph) << "Should delete def" << defload.name << " after conversion it has no children, nor models";
        return false;
    }

    postprocessNodeFlags(defload,node);
    postprocessLOD(defload.p_Lod, node);
    postprocessTextureReplacers(defload.p_ReplaceTex,node);
    postprocessTintColor(defload.p_TintColor,node);
    postprocessAmbient(defload.p_Ambient,node);
    postprocessFog(defload.p_Fog,node);
    postprocessEditorBeacon(defload.p_Beacon, node);
    postprocessSound(defload.p_Sound,node);
    postprocessLight(defload.p_Omni, node);

    if(!defload.type.isEmpty())
        node->m_fx_name_hash  = CompileTimeUtils::hash_32_fnv1a_const(defload.type.toLower().constData());

    nodeCalculateBounds(node);
    nodeSetVisBounds(node);
    return true;
}

void serializeIn(SceneGraph_Data &scenegraph,LoadingContext &ctx,PrefabStore &prefabs)
{
    for (const SceneGraphNode_Data & node_dat : scenegraph.Def )
        addNode(node_dat, ctx,prefabs);
    for (const SceneRootNode_Data & root_dat : scenegraph.Ref)
        addRoot(root_dat, ctx,prefabs);
}

namespace SEGS {

bool loadSceneGraph(const QString &path,LoadingContext &ctx,PrefabStore &prefabs)
{
    QString binName = mapNameToPath(path,ctx);
    SceneGraph_Data serialized_graph;
    ctx.m_renamer.basename = buildBaseName(path);
    binName.replace(QStringLiteral("Chunks.bin"), QStringLiteral("CHUNKS.bin"));
    LoadSceneData(binName, serialized_graph);

    serializeIn(serialized_graph, ctx, prefabs);
    return true;
}

SceneGraph *loadWholeMap(const QString &filename)
{
    RuntimeData &rd(getRuntimeData());

    SceneGraph *m_scene_graph = new SceneGraph;
    SEGS::LoadingContext ctx;
    ctx.m_target = m_scene_graph;
    int geobin_idx= filename.indexOf("geobin");
    int maps_idx = filename.indexOf("maps");
    ctx.m_base_path = filename.mid(0, geobin_idx);
    assert(rd.m_prefab_mapping);
    QString upcase_city = filename;
    upcase_city.replace("city", "City");
    upcase_city.replace("hazard", "Hazard");
    upcase_city.replace("trial", "Trial");
    upcase_city.replace("zones", "Zones");
    rd.m_prefab_mapping->sceneGraphWasReset();
    bool res = loadSceneGraph(upcase_city.mid(maps_idx), ctx, *rd.m_prefab_mapping);
    if(!res)
    {
        delete m_scene_graph;
        return nullptr;
    }
    return m_scene_graph;
}

void loadSubgraph(const QString &filename, LoadingContext &ctx,PrefabStore &prefabs)
{
    QFileInfo fi(filename);
    LoadingContext tmp = ctx;
    loadSceneGraph(fi.path()+"/"+fi.completeBaseName()+".txt",tmp,prefabs);
}

} // end of SEGS namespace
