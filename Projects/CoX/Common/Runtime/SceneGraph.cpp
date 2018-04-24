#include "SceneGraph.h"

#include "GameData/scenegraph_definitions.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "GameData/DataStorage.h"
#include "GameData/CoHMath.h"
#include "Logging.h"

#include "glm/mat3x3.hpp"
#include "glm/gtx/quaternion.hpp"
#include <QDir>
#include <cmath>



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
struct GeoSet
{
    QString              geopath;
    QString              name;
    GeoSet *             parent_geoset = nullptr;
    std::vector<Model *> subs;
    std::vector<QString> tex_names;
    std::vector<char>    m_geo_data;
    uint32_t             geo_data_size;
    bool                 data_loaded = false;
};
QHash<QString,GeoSet *> s_name_to_geoset;
AllTricks_Data s_tricks_store;
QHash<QString,TextureModifiers *> g_texture_path_to_mod;
QHash<QString,GeometryModifiers *> g_tricks_string_hash_tab;

inline QByteArray uncompr_zip(char *comp_data,int size_comprs,uint32_t size_uncom)
{
    QByteArray compressed_data;
    compressed_data.reserve(size_comprs+4);
    compressed_data.append( char((size_uncom >> 24) & 0xFF));
    compressed_data.append( char((size_uncom >> 16) & 0xFF));
    compressed_data.append( char((size_uncom >> 8) & 0xFF));
    compressed_data.append( char((size_uncom >> 0) & 0xFF));
    compressed_data.append(comp_data,size_comprs);
    return qUncompress(compressed_data);
}
Model *convertAndInsertModel(GeoSet &tgt, const Model32 *v)
{
    Model *z = new Model;

    z->flags = v->flg1;
    z->visibility_radius = v->radius;
    z->num_textures = v->num_textures;
    //z->boneinfo_offset = v->boneinfo;
    //z->blend_mode = CoHBlendMode(v->blend_mode);
    //z->vertex_count = v->vertex_count;
    //z->model_tri_count = v->model_tri_count;
    z->scale = v->m_scale;
    z->box.m_min = v->m_min;
    z->box.m_max = v->m_max;
    /*
    for (uint8_t i = 0; i < 7; ++i)
    {
        DeltaPack &dp_blk(z->packed_data[i]);
        dp_blk.compressed_size = v->pack_data[i].compressed_size;
        dp_blk.uncomp_size = v->pack_data[i].uncomp_size;
        dp_blk.compressed_data = nullptr;
        dp_blk.buffer_offset = v->pack_data[i].compressed_data_off;
    }*/
    tgt.subs.push_back(z);
    return z;
}
void geosetLoadHeader(QFile &fp, GeoSet *geoset)
{
    unsigned int anm_hdr_size;
    const uint8_t * stream_pos_0;
    const uint8_t * stream_pos_1;
    uint32_t headersize;
    fp.read((char *)&anm_hdr_size, 4u);
    anm_hdr_size -= 4;
    fp.read((char *)&headersize, sizeof(uint32_t));

    QByteArray zipmem = fp.read(anm_hdr_size);
    QByteArray unc_arr = uncompr_zip(zipmem.data(), anm_hdr_size, headersize);

    const uint8_t * mem = (const uint8_t *)unc_arr.data();

    const TexBlockInfo *info = (const TexBlockInfo *)mem;
    geoset->geo_data_size = info->size1;

    //convertTextureNames((const int *)(mem + sizeof(TexBlockInfo)), geoset->tex_names);
    stream_pos_0            = mem + info->texname_blocksize + sizeof(TexBlockInfo);
    stream_pos_1            = stream_pos_0 + info->bone_names_size;
    const GeosetHeader32 *header32  = (const GeosetHeader32 *)(stream_pos_1 + info->tex_binds_size);
    const Model32 *     ptr_subs  = (Model32 *)(stream_pos_1 + info->tex_binds_size + sizeof(GeosetHeader32));
    geoset->parent_geoset = geoset;
    geoset->name = header32->name;
    for (int idx = 0; idx < header32->num_subs; ++idx)
    {
        const Model32 *v6 = &ptr_subs[idx];
        //std::vector<TextureBind> binds;
        //if (info->tex_binds_size) {
        //    binds = convertTexBinds(v6->num_textures, v6->texture_bind_offsets + stream_pos_1);
        //}
        Model *m    = convertAndInsertModel(*geoset, v6);
        //m->texture_bind_info = binds;
        //m->geoset       = geoset;
        m->name         = QString((const char *)stream_pos_0 + v6->bone_name_offset);
    }
//    if (!geoset->subs.empty())
//        addModelStubs(geoset);
}
GeoSet *findAndPrepareGeoSet(const QString &fname,LoadingContext &ctx)
{
    GeoSet *geoset = nullptr;
    QFile fp;
    fp.setFileName(ctx.m_base_path+fname);
    if (fp.open(QFile::ReadOnly))
    {
        geoset = new GeoSet;
        geoset->geopath = fname;
        geosetLoadHeader(fp, geoset);
        fp.seek(0);
        s_name_to_geoset[fname] = geoset;
    } else {
        qCritical() <<"Can't find .geo file"<<fname;
    }
    return geoset;
}
/// load the given geoset, used when loading scene-subgraph and nodes
GeoSet * geosetLoad(const QString &m,LoadingContext &ctx)
{
    GeoSet * res = s_name_to_geoset.value(m,nullptr);
    if(res)
        return res;
    return findAndPrepareGeoSet(m,ctx);
}

bool groupInLibSub(const QString &name)
{
    if(name.contains('/')) {
        return !name.startsWith("maps");
    }
    return !name.startsWith("grp");
}
SceneNode * getNodeByName(const SceneGraph &graph,const QString &name)
{
    QString filename;
    int idx = name.lastIndexOf('/');
    if ( idx==-1 )
        filename = name;
    else
        filename = name.mid(idx+1);
    return graph.name_to_node.value(filename.toLower(),nullptr);
}
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
    if ( groupInLibSub(str) )
        return str;
    if ( !is_def && !str.contains("/grp",Qt::CaseInsensitive) && !str.contains("/map",Qt::CaseInsensitive) )
        return str;
    QString querystring = QString(str).toLower();
    auto str_iter = ctx.m_renamer.new_names.find(querystring);

    if ( str_iter!=ctx.m_renamer.new_names.end() )
        return *str_iter;

    QString prefix = str;
    int gidx = prefix.indexOf("/grp",0,Qt::CaseInsensitive);
    if ( gidx!=-1 )
    {
        prefix = prefix.mid(gidx+4); // skip /grp
        prefix = prefix.mid(0,prefix.indexOf(QRegExp("[^A-Za-z]"))); // collect chars to first non-letter
    }
    else
    {
        if ( prefix.contains("/map",Qt::CaseInsensitive) )
            prefix = "maps/grp";
        else
        {
            qDebug() << "bad def name:" <<prefix;
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
    if ( -1==start_idx )
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
    if(idx>=scene.refs.size()) {
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
    if(!def) {
        if(store.loadNamedPrefab(newname,ctx))
            def = getNodeByName(*ctx.m_target,newname);
    }
    if(!def) {
        qCritical() << "Missing reference:"<<newname;
        return;
    }
    auto ref = newRef(*ctx.m_target);
    ref->node = def;
    rotationFromYPR(ref->mat,{refload.rot.x,refload.rot.y,refload.rot.z});
    ref->mat[3] = glm::vec4(refload.pos,1);
}
SceneNode *newDef(SceneGraph &scene)
{
    SceneNode *res = new SceneNode;
    res->m_index_in_scenegraph = scene.all_converted_defs.size();
    scene.all_converted_defs.emplace_back(res);
    return res;
}
void setNodeNameAndPath(SceneGraph &scene,SceneNode *node, QString obj_path)
{
    QString result;
    size_t strlenobjec = strlen("object_library");
    if ( obj_path.startsWith("object_library", Qt::CaseInsensitive) )
        obj_path.remove(0, strlenobjec + 1);
    if ( groupInLibSub(obj_path) )
        result = "object_library/";
    result += obj_path;
    int last_separator = result.lastIndexOf('/');
    QStringRef key = result.midRef(last_separator + 1);
    QString lowkey = key.toString().toLower();
    auto iter = scene.name_to_node.find(lowkey);
    if(iter==scene.name_to_node.end()) {
        iter=scene.name_to_node.insert(lowkey,node);
    }
    node->name = iter.key();
    node->dir.clear();
    if ( key.position() != 0 )
        node->dir = result.mid(0,key.position()-1);
}
void addChildNodes(const SceneGraphNode_Data &inp_data, SceneNode *node, LoadingContext &ctx, PrefabStore &store)
{
    if ( inp_data.p_Grp.empty() )
        return;
    node->children.reserve(inp_data.p_Grp.size());
    for(const GroupLoc_Data & dat : inp_data.p_Grp)
    {
        const QString new_name = groupRename(ctx, dat.name, false);
        SceneNodeChildTransform child;
        child.node = getNodeByName(*ctx.m_target,new_name);
        if ( !child.node )
        {
            store.loadNamedPrefab(new_name,ctx);
            child.node = getNodeByName(*ctx.m_target,new_name);
        }
        // construct from euler angles
        glm::quat qPitch = glm::angleAxis(dat.rot.x, glm::vec3(-1, 0, 0));
        glm::quat qYaw =  glm::angleAxis(dat.rot.y, glm::vec3(0, 1, 0));
        glm::quat qRoll = glm::angleAxis(dat.rot.z, glm::vec3(0, 0, 1));
        glm::quat rotQuat = qYaw * qPitch * qRoll;
        child.m_matrix2 = glm::mat3(rotQuat);
        child.m_translation = dat.pos;
        if ( child.node )
            node->children.emplace_back(child);
        else
        {
            qCritical() << "Node"<<node->name<<"\ncan't find member" << dat.name;
        }
    }
}
void addLod(const std::vector<DefLod_Data> &a1, SceneNode *a2)
{
    if(a1.empty())
        return;
    const DefLod_Data &v2(a1.front());
    a2->lod_scale = v2.Scale;

    if ( a2->lod_fromtrick )
        return;

    a2->lod_far       = v2.Far;
    a2->lod_far_fade  = v2.FarFade;
    a2->lod_near      = v2.Near;
    a2->lod_near_fade = v2.NearFade;
}
bool nodeCalculateBounds(SceneNode *group)
{
    float geometry_radius=0.0f;
    float maxrad=0.0f;
    Model *model;
    AxisAlignedBoundingBox bbox;
    bool set = false;
    if ( !group )
        return false;
    if ( group->model )
    {
        model = group->model;
        bbox.merge(model->box);

        geometry_radius = glm::length(bbox.size()) * 0.5f;
        set = true;
    }
    for ( SceneNodeChildTransform & child : group->children )
    {
        glm::vec3 dst(child.node->center * child.m_matrix2 + child.m_translation);
        glm::vec3 v_radius(child.node->radius,child.node->radius,child.node->radius);
        bbox.merge(dst+v_radius);
        bbox.merge(dst-v_radius);
        set = true;
    }
    if ( !set )
    {
        bbox.clear();
    }
    group->radius = glm::length(bbox.size()) * 0.5f;
    group->m_bbox = bbox;
    group->center = bbox.center(); // center
    for ( SceneNodeChildTransform & child : group->children )
    {
        glm::vec3 toChildCenter = (child.m_matrix2 * child.node->center + child.m_translation) - group->center;
        float r = glm::length(toChildCenter) + child.node->radius;
        maxrad = std::max(maxrad,r);
    }
    if ( maxrad != 0.0f )
        group->radius = maxrad;
    group->radius = std::max(geometry_radius,group->radius);
    return group->radius == 0.0f && !group->children.empty();
}
void  nodeSetVisBounds(SceneNode *group)
{
    //TODO: fix this
    glm::vec3 dv;
    float maxrad = 0.0;
    float maxvis = 0.0;
    if ( !group )
        return;
    if ( group->lod_scale == 0.0f )
        group->lod_scale = 1.0f;
    if ( group->model )
    {
        Model *v1 = group->model;
        dv = v1->box.size();
        maxrad = glm::length(dv) * 0.5f + group->shadow_dist;
        if ( group->lod_far == 0.0f )
        {
            group->lod_far = (maxrad + 10.0f) * 10.0f;
            group->lod_far_fade = group->lod_far * 0.25f;
            group->lod_autogen = true;
        }
        maxvis = group->lod_far + group->lod_far_fade;
    }
    for (SceneNodeChildTransform &entr : group->children)
    {
        dv = entr.m_matrix2 * entr.node->center + entr.m_translation;
        dv -= group->center;
        maxrad = std::max(maxrad,glm::length(dv) + entr.node->radius + entr.node->shadow_dist);
        maxvis = std::max(maxvis,glm::length(dv) + entr.node->vis_dist * entr.node->lod_scale);
    }
    if ( group->shadow_dist == 0.0f )
        group->shadow_dist = maxrad - group->radius;
    group->vis_dist = maxvis;
}

bool addNode(const SceneGraphNode_Data &defload, LoadingContext &ctx,PrefabStore &prefabs)
{
    if (defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;
    QString obj_path = groupRename(ctx, defload.name, true);
    SceneNode * node = getNodeByName(*ctx.m_target,obj_path);
    if (!node)
    {
        node = newDef(*ctx.m_target);
        if (!defload.p_Property.empty())
            node->properties = new std::vector<GroupProperty_Data> (defload.p_Property);
    }
    if ( !defload.p_Obj.isEmpty() )
    {
        node->model = prefabs.groupModelFind(defload.p_Obj,ctx);
        if ( !node->model )
        {
            qCritical() << "Cannot find root geometry in" << defload.p_Obj;
        }
        //groupApplyModifiers(node);
    }
    setNodeNameAndPath(*ctx.m_target,node,obj_path);
    addChildNodes(defload,node,ctx,prefabs);

    if ( node->children.empty() && !node->model )
    {
        qDebug() << "Should delete def"<<defload.name<<" after conversion it has no children, nor models";
        return false;
    }
    addLod(defload.p_Lod, node);

    nodeCalculateBounds(node);
    nodeSetVisBounds(node);
    return true;
}

}

void serializeIn(SceneGraph_Data &scenegraph,LoadingContext &ctx,PrefabStore &prefabs)
{
    for (const SceneGraphNode_Data & node_dat : scenegraph.Def )
        addNode(node_dat, ctx,prefabs);
    for (const SceneRootNode_Data & root_dat : scenegraph.Ref)
        addRoot(root_dat, ctx,prefabs);
}
bool loadSceneGraph(const QString &path,LoadingContext &ctx,PrefabStore &prefabs)
{
    QString binName = mapNameToPath(path,ctx);
    SceneGraph_Data serialized_graph;
    ctx.m_renamer.basename = buildBaseName(path);
    binName.replace("CHUNKS.bin","Chunks.bin");
    LoadSceneData(binName, serialized_graph);

    serializeIn(serialized_graph, ctx, prefabs);
    return true;
}
void loadSubgraph(const QString &filename, LoadingContext &ctx,PrefabStore &prefabs)
{
    geosetLoad(filename,ctx); // load given subgraph's root geoset
    QFileInfo fi(filename);
    LoadingContext tmp = ctx;
    loadSceneGraph(fi.path()+"/"+fi.completeBaseName()+".txt",tmp,prefabs);
}

GeoStoreDef * PrefabStore::groupGetFileEntryPtr(const QString &a1)
{
    QString key = a1.mid(a1.lastIndexOf('/') + 1);
    key = key.mid(0, key.indexOf("__"));
    return m_modelname_to_geostore.value(key, nullptr);
}
static void registerGeometryModifier(GeometryModifiers *a1)
{
    if (a1->node.TintColor0.rgb_are_zero())
        a1->node.TintColor0 = RGBA(0xFFFFFFFF);
    if (a1->node.TintColor1.rgb_are_zero())
        a1->node.TintColor1 = RGBA(0xFFFFFFFF);
    a1->AlphaRef /= 255.0f;
    if (a1->ObjTexBias != 0.0f)
        a1->node._TrickFlags |= TexBias;
    if (a1->AlphaRef != 0.0f)
        a1->node._TrickFlags |= AlphaRef;
    if (a1->FogDist.x != 0.0f || a1->FogDist.y != 0.0f)
        a1->node._TrickFlags |= FogHasStartAndEnd;
    if (a1->ShadowDist != 0.0f)
        a1->node._TrickFlags |= CastShadow;
    if (a1->NightGlow.x != 0.0f || a1->NightGlow.y != 0.0f)
        a1->node._TrickFlags |= NightGlow;
    if (a1->node.ScrollST0.x != 0.0f || a1->node.ScrollST0.y != 0.0f)
        a1->node._TrickFlags |= ScrollST0;
    if (a1->node.ScrollST1.x != 0.0f || a1->node.ScrollST1.y != 0.0f)
        a1->node._TrickFlags |= ScrollST1;
    if (!a1->StAnim.empty())
    {
        //        if (setStAnim(&a1->StAnim.front()))
        //            a1->node._TrickFlags |= STAnimate;
    }
    if (a1->GroupFlags & VisTray)
        a1->ObjFlags |= 0x400;
    if (a1->name.isEmpty())
        qDebug() << "No name in trick";
    auto iter = g_tricks_string_hash_tab.find(a1->name.toLower());
    if (iter!=g_tricks_string_hash_tab.end())
    {
        qDebug() << "duplicate model trick!";
        return;
    }
    g_tricks_string_hash_tab[a1->name.toLower()]=a1;
}

static void setupTexOpt(TextureModifiers *tex)
{
    if (tex->ScaleST0.x == 0.0f)
        tex->ScaleST0.x = 1.0f;
    if (tex->ScaleST0.y == 0.0f)
        tex->ScaleST0.y = 1.0f;
    if (tex->ScaleST1.x == 0.0f)
        tex->ScaleST1.x = 1.0f;
    if (tex->ScaleST1.y == 0.0f)
        tex->ScaleST1.y = 1.0f;
    if (tex->Fade.x != 0.0f || tex->Fade.y != 0.0f)
        tex->Flags |= uint32_t(TexOpt::FADE);
    if (!tex->Blend.isEmpty())
        tex->Flags |= uint32_t(TexOpt::DUAL);
    if (!tex->Surface.isEmpty())
    {
        //qDebug() <<"Has surface"<<tex->Surface;
    }

    tex->name = tex->name.mid(0,tex->name.lastIndexOf('.')); // cut last extension part
    if(tex->name.startsWith('/'))
        tex->name.remove(0,1);
    auto iter = g_texture_path_to_mod.find(tex->name.toLower());
    if (iter!=g_texture_path_to_mod.end())
    {
        qDebug() << "duplicate texture info: "<<tex->name;
        return;
    }
    g_texture_path_to_mod[tex->name.toLower()] = tex;
}
static void  trickLoadPostProcess(AllTricks_Data *a2)
{
    g_texture_path_to_mod.clear();
    g_tricks_string_hash_tab.clear();
    for (TextureModifiers &texopt : a2->texture_mods)
        setupTexOpt(&texopt);
    for (GeometryModifiers &trickinfo : a2->geometry_mods)
        registerGeometryModifier(&trickinfo);
}
bool loadTricksBin(const QString &base_path)
{
    BinStore binfile;
    QString fname(base_path+"bin/tricks.bin");
    if(!LoadModifiersData(fname, s_tricks_store))
    {
        return false;
    }
    trickLoadPostProcess(&s_tricks_store);
    return true;
}
bool PrefabStore::prepareGeoLookupArray(const QString &base_path)
{
    QFile defnames(base_path + "bin/defnames.bin");
    if (!defnames.open(QFile::ReadOnly))
    {
        qCritical() << "Failed to open defnames.bin";
        return false;
    }
    QByteArrayList defnames_arr;
    for (const QByteArray &entr : defnames.readAll().split('\0'))
        defnames_arr.push_back(entr);
    QString lookup_str;
    GeoStoreDef *current_geosetinf = nullptr;
    for (QString str : defnames_arr)
    {
        str.replace("CHUNKS.geo", "Chunks.geo"); // original paths are case insensitive
        int last_slash = str.lastIndexOf('/');
        if (-1 != last_slash)
        {
            QString geo_path = str.mid(0, last_slash);
            lookup_str = geo_path.toLower();
            current_geosetinf = &m_dir_to_geoset[lookup_str];
            current_geosetinf->geopath = geo_path;
        }
        current_geosetinf->entries << str.mid(last_slash + 1);
        m_modelname_to_geostore[str.mid(last_slash + 1)] = current_geosetinf;
    }
    return loadTricksBin(base_path);
}

bool PrefabStore::loadPrefabForNode(SceneNode *node, LoadingContext &ctx) //groupLoadRequiredLibsForNode
{
    GeoStoreDef *gf;

    if (!node || !node->in_use)
        return false;

    if (node->geoset_info)
        gf = node->geoset_info;
    else
    {
        gf = groupGetFileEntryPtr(node->name);
        node->geoset_info = gf;
        if (!node->geoset_info)
            node->geoset_info = (GeoStoreDef *)-1; // prevent future load attempts
    }
    if (!gf || gf == (GeoStoreDef *)-1)
        return false;
    if (!gf->loaded)
    {
        gf->loaded = true;
        loadSubgraph(gf->geopath,ctx,*this);
    }
    return true;
}
bool PrefabStore::loadNamedPrefab(const QString &name, LoadingContext &ctx) //groupFileLoadFromName
{
    GeoStoreDef *geo_store = groupGetFileEntryPtr(name);
    if (!geo_store)
        return false;
    if (geo_store->loaded)
        return true;

    geo_store->loaded = true;
    loadSubgraph(geo_store->geopath,ctx,*this);
    loadPrefabForNode(getNodeByName(*ctx.m_target,name), ctx);
    return true;
}
Model *modelFind(const QString &geoset_name, const QString &model_name,LoadingContext &ctx)
{
    Model *ptr_sub = nullptr;

    if (model_name.isEmpty() || geoset_name.isEmpty()) {
        qCritical() << "Bad model/geometry set requested:";
        if (!model_name.isEmpty())
            qCritical() << "Model: " << model_name;
        if (!geoset_name.isEmpty())
            qCritical() << "GeoFile: " << geoset_name;
        return nullptr;
    }
    GeoSet *geoset = geosetLoad(geoset_name,ctx);
    if (!geoset) // failed to load the geometry set
        return nullptr;
    int end_of_name_idx = model_name.indexOf("__");
    if (end_of_name_idx == -1)
        end_of_name_idx = model_name.size();
    QStringRef basename(model_name.midRef(0, end_of_name_idx));

    for (Model *m : geoset->subs)
    {
        QString geo_name = m->name;
        if (geo_name.isEmpty())
            continue;
        bool subs_in_place = (geo_name.size() <= end_of_name_idx || geo_name.midRef(end_of_name_idx).startsWith("__"));
        if (subs_in_place && geo_name.startsWith(basename, Qt::CaseInsensitive))
            ptr_sub = m; // TODO: return immediately
    }
    return ptr_sub;

}
Model *PrefabStore::groupModelFind(const QString & path,LoadingContext &ctx)
{
    QString model_name = path.mid(path.lastIndexOf('/') + 1);
    auto val = groupGetFileEntryPtr(model_name);
    return val ? modelFind(val->geopath, model_name,ctx) : nullptr;
}
