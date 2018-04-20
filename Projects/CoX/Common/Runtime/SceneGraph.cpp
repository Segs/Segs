#include "SceneGraph.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/DataStorage.h"
#include "Logging.h"

#include "glm/mat3x3.hpp"
#include "glm/gtx/quaternion.hpp"
#include <QDir>
#include <cmath>

// Geo file info
struct GeoStoreDef
{
    QString geopath;        //!< a path to a .geo file
    QStringList entries;    //!< the names of models contained in a geoset
    bool loaded;
};
// Node name re-mapping support
struct NameList
{
    QHash<QString,QString> new_names; // map from old node name to a new name
    QString basename;
};

namespace
{
QString g_basepath;
int last_node_id=0; // used to create new number suffixes for generic nodes
QHash<QString,SceneNode *> name_to_node; // working name to node mapping.
QHash<QString,GeoStoreDef *> s_modelname_to_geostore;
GeoStoreDef * groupGetFileEntryPtr(const QString &a1)
{
    QString key = a1.mid(a1.lastIndexOf('/')+1);
    key = key.mid(0,key.indexOf("__"));
    return s_modelname_to_geostore.value(key,nullptr);
}


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
        //Model *m    = convertAndInsertModel(*geoset, v6);
        //m->texture_bind_info = binds;
        //m->geoset       = geoset;
        //m->name         = QString((const char *)stream_pos_0 + v6->bone_name_offset);
    }
//    if (!geoset->subs.empty())
//        addModelStubs(geoset);
}
static GeoSet *findAndPrepareGeoSet(const QString &fname)
{
    GeoSet *geoset = nullptr;
    QFile fp;
    fp.setFileName(g_basepath+fname);
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
GeoSet * geosetLoad(const QString &m)
{
    GeoSet * res = s_name_to_geoset.value(m,nullptr);
    if(res)
        return res;
    return findAndPrepareGeoSet(m);
}
NameList my_name_list;


void rotationFromYPR(glm::mat4x3 & mat, const glm::vec3 &pyr)
{
    float   cos_p     =  std::cos(pyr.x);
    float   neg_sin_p = -std::sin(pyr.x);
    float   cos_y     =  std::cos(pyr.y);
    float   neg_sin_y = -std::sin(pyr.y);
    float   cos_r     =  std::cos(pyr.z);
    float   neg_sin_r = -std::sin(pyr.z);
    float   tmp       =  cos_y * neg_sin_p;
    glm::mat3 rotmat;
    rotmat[0][0] = cos_r * cos_y - neg_sin_y * neg_sin_p * neg_sin_r;
    rotmat[0][1] = neg_sin_r * cos_p;
    rotmat[0][2] = tmp * neg_sin_r + cos_r * neg_sin_y;
    rotmat[1][0] = -(neg_sin_r * cos_y) - neg_sin_y * neg_sin_p * cos_r;
    rotmat[1][1] = cos_r * cos_p;
    rotmat[1][2] = tmp * cos_r - neg_sin_r * neg_sin_y;
    rotmat[2][0] = -(neg_sin_y * cos_p);
    rotmat[2][1] = -neg_sin_p;
    rotmat[2][2] = cos_y * cos_p;

    mat[0]=rotmat[0];
    mat[1]=rotmat[1];
    mat[2]=rotmat[2];
}
bool groupInLibSub(const QString &name)
{
    if(name.contains('/')) {
        return !name.startsWith("maps");
    }
    return !name.startsWith("grp");
}
SceneNode * getNodeByName(const QString &name)
{
    QString filename;
    int idx = name.lastIndexOf('/');
    if ( idx==-1 )
        filename = name;
    else
        filename = name.mid(idx+1);
    return name_to_node.value(filename.toLower(),nullptr);
}
QString  groupMakeName(const QString &base)
{
    QString buf;
    do
        buf = base + QString::number(last_node_id);
    while (getNodeByName(buf));
    return buf;
}
// Create new names for any 'numbered' scene nodes
QString groupRename(NameList &memory, const QString &oldname, bool is_def)
{
    QString str;
    str = oldname.contains('/') ? oldname : memory.basename+'/'+oldname;
    if ( groupInLibSub(str) )
        return str;
    if ( !is_def && !str.contains("/grp",Qt::CaseInsensitive) && !str.contains("/map",Qt::CaseInsensitive) )
        return str;
    QString querystring = QString(str).toLower();
    auto str_iter = memory.new_names.find(querystring);
    if ( str_iter!=memory.new_names.end() ) {
        return *str_iter;
    }
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
    QString tgt = groupMakeName(prefix);
    memory.new_names[querystring] = tgt;
    return tgt;
}
QString buildBaseName(QString path)
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

QString mapNameToPath(const QString &name)
{
    int start_idx = name.indexOf("object_library",Qt::CaseInsensitive);
    if ( -1==start_idx )
        start_idx = name.indexOf("maps",Qt::CaseInsensitive);
    QString buf = g_basepath +"geobin/" + name.midRef(start_idx);
    const int last_dot = buf.lastIndexOf('.');
    if(-1==last_dot)
        buf+=".bin";
    else if(!buf.contains(".crl"))
        buf.replace(last_dot,buf.size()-last_dot,".bin");
    return buf;
}
bool LoadSceneData(const QString &fname, SceneGraph_Data &scenegraph)
{
    BinStore binfile;

    if (fname.contains(".crl"))
    {
        if (!loadFrom(fname, scenegraph))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
        return true;
    }
    if (!binfile.open(fname, scenegraph_i0_2_requiredCrc))
    {
        qCritical() << "Failed to open original bin:" << fname;
        return false;
    }
    if (!loadFrom(&binfile, scenegraph))
    {
        qCritical() << "Failed to load data from original bin:" << fname;
        return false;
    }
    return true;
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

bool groupLoadRequiredLibsForNode(SceneNode *node,SceneGraph &conv)
{
    GeoStoreDef *gf;

    if ( !node || !node->in_use )
        return false;

    if ( node->geoset_info )
        gf = node->geoset_info;
    else
    {
        gf = groupGetFileEntryPtr(node->name);
        node->geoset_info = gf;
        if ( !node->geoset_info )
            node->geoset_info = (GeoStoreDef *)-1; // prevent future load attempts
    }
    if ( !gf || gf == (GeoStoreDef *)-1 )
        return false;
    if ( !gf->loaded )
    {
        gf->loaded = true;
        conv.loadSubgraph(gf->geopath);
    }
    return true;
}
bool groupFileLoadFromName(const QString &name,SceneGraph &conv)
{
    GeoStoreDef *geo_store = groupGetFileEntryPtr(name);
    if ( !geo_store )
        return false;
    if ( geo_store->loaded )
        return true;

    geo_store->loaded = true;
    conv.loadSubgraph(geo_store->geopath);
    groupLoadRequiredLibsForNode(getNodeByName(name),conv);
    return 1;
}
void addRoot(const SceneRootNode_Data &refload, NameList &namelist,SceneGraph &conv)
{
    QString newname = groupRename(namelist, refload.name, 0);
    auto *def = getNodeByName(newname);
    if(!def) {
        groupFileLoadFromName(newname,conv);
        def = getNodeByName(newname);
    }
    if(!def) {
        qCritical() << "Missing reference:"<<newname;
        return;
    }
    auto ref = newRef(conv);
    ref->node = def;
    rotationFromYPR(ref->mat,{refload.rot.x,refload.rot.y,refload.rot.z});
    ref->mat[3] = refload.pos;
}
SceneNode *newDef(SceneGraph &scene)
{
    SceneNode *res = new SceneNode;
    scene.all_converted_defs.emplace_back(res);
    return res;
}
Model *modelFind(const QString &geoset_name,const QString &model_name)
{
    Model *ptr_sub = nullptr;

    if (model_name.isEmpty() || geoset_name.isEmpty()) {
        qCritical() << "Bad model/geometry set requested:";
        if (!model_name.isEmpty())
            qCritical() << "Model: "<<model_name;
        if (!geoset_name.isEmpty())
            qCritical() << "GeoFile: "<<geoset_name;
        return nullptr;
    }
    GeoSet *geoset = geosetLoad(geoset_name);
    if (!geoset) // failed to load the geometry set
        return nullptr;
    int end_of_name_idx  = model_name.indexOf("__");
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
Model *groupModelFind(const QString & path)
{
    QString model_name=path.mid(path.lastIndexOf('/')+1);
    auto val = groupGetFileEntryPtr(model_name);
    return val ? modelFind(val->geopath,model_name) : nullptr;
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
    int v4 = result.lastIndexOf('/');
    QStringRef key = result.midRef(v4 + 1);
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
void addChildNodes(const SceneGraphNode_Data &a1, SceneNode *node,NameList &a3,SceneGraph &conv)
{
    if ( a1.p_Grp.empty() )
        return;
    node->children.reserve(a1.p_Grp.size());
    for(const GroupLoc_Data & dat : a1.p_Grp)
    {
        QString v5 = groupRename(a3, dat.name, 0);
        SceneNodeChildTransform child;
        child.node = getNodeByName(v5);
        if ( !child.node )
        {
            groupFileLoadFromName(v5,conv);
            child.node = getNodeByName(v5);
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
    bool set = 0;
    if ( !group )
        return false;
    if ( group->model )
    {
        model = group->model;
        bbox.merge(model->box);

        geometry_radius = glm::length(bbox.size()) * 0.5f;
        set = 1;
    }
    for ( SceneNodeChildTransform & child : group->children )
    {
        glm::vec3 dst(child.node->center * child.m_matrix2 + child.m_translation);
        glm::vec3 v_radius(child.node->radius,child.node->radius,child.node->radius);
        bbox.merge(dst+v_radius);
        bbox.merge(dst-v_radius);
        set = 1;
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
            group->lod_autogen = 1;
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

bool addNode(const SceneGraphNode_Data &defload, NameList &renamer,SceneGraph &conv)
{
    if (defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;
    QString obj_path = groupRename(renamer, defload.name, 1);
    SceneNode * node = getNodeByName(obj_path);
    if ( !node )
        node = newDef(conv);
    if ( !defload.p_Obj.isEmpty() )
    {
        node->model = groupModelFind(defload.p_Obj);
        if ( !node->model )
        {
            qCritical() << "Cannot find root geometry in" << defload.p_Obj;
        }
        //groupApplyModifiers(node);
    }
    setNodeNameAndPath(conv,node,obj_path);
    addChildNodes(defload,node,renamer,conv);

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

void SceneGraph::serializeIn(SceneGraph_Data &scenegraph,NameList &renamer)
{
    for (const SceneGraphNode_Data & node_dat : scenegraph.Def )
        addNode(node_dat, renamer,*this);
    for (const SceneRootNode_Data & root_dat : scenegraph.Ref)
        addRoot(root_dat, renamer,*this);
}
bool SceneGraph::loadSceneGraph(const QString &path)
{
    name_to_node.clear(); // reset local mapping
    QString binName = mapNameToPath(path);
    NameList my_name_list;
    my_name_list.basename = buildBaseName(path);
    SceneGraph_Data scenegraph;
    binName.replace("CHUNKS.bin","Chunks.bin");
    LoadSceneData(binName,scenegraph);

    serializeIn(scenegraph,my_name_list);
    name_to_node = std::move(name_to_node);
    return true;
}
void SceneGraph::loadSubgraph(const QString &filename)
{
    geosetLoad(filename); // load given subgraph's root geoset
    QFileInfo fi(filename);
    loadSceneGraph(fi.path()+"/"+fi.completeBaseName()+".txt");
}
