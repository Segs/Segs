#include "CoHSceneConverter.h"
#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/trick_definitions.h"

#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/BoundingBox.h>
#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Resource/Image.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/UI/Font.h>
#include <Lutefisk3D/UI/Text3D.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <vector>

using namespace Urho3D;
extern QString basepath;
namespace
{
glm::vec3 fromUrho(Vector3 v) { return {v.x_,v.y_,v.z_};}
Vector3 toUrho(glm::vec3 v) { return {v.x,v.y,v.z};}
struct NameList
{
    QHash<QString,QString> new_names; // map from old node name to a new name
    QString basename;
};
static NameList my_name_list;

bool groupFileLoadFromName(CoHSceneGraph &conv,const QString &a1);

void rotationFromYPR(Matrix3x4 & mat, const Vector3 &pyr)
{
    float   cos_p     =  std::cos(pyr.x_);
    float   neg_sin_p = -std::sin(pyr.x_);
    float   cos_y     =  std::cos(pyr.y_);
    float   neg_sin_y = -std::sin(pyr.y_);
    float   cos_r     =  std::cos(pyr.z_);
    float   neg_sin_r = -std::sin(pyr.z_);
    float   tmp       =  cos_y * neg_sin_p;
    Matrix3 rotmat;
    rotmat.m00_ = cos_r * cos_y - neg_sin_y * neg_sin_p * neg_sin_r;
    rotmat.m01_ = neg_sin_r * cos_p;
    rotmat.m02_ = tmp * neg_sin_r + cos_r * neg_sin_y;
    rotmat.m10_ = -(neg_sin_r * cos_y) - neg_sin_y * neg_sin_p * cos_r;
    rotmat.m11_ = cos_r * cos_p;
    rotmat.m12_ = tmp * cos_r - neg_sin_r * neg_sin_y;
    rotmat.m20_ = -(neg_sin_y * cos_p);
    rotmat.m21_ = -neg_sin_p;
    rotmat.m22_ = cos_y * cos_p;
    mat.SetRotation(rotmat);
}

CoHNode *newDef(CoHSceneGraph &scene)
{
    CoHNode *res = new CoHNode;
    scene.all_converted_defs.emplace_back(res);
    return res;
}
ConvertedRootNode *newRef(CoHSceneGraph &scene)
{
    size_t idx;
    for(idx=0; idx<scene.refs.size(); ++idx)
        if(!scene.refs[idx] || !scene.refs[idx]->node)
            break;
    if(idx>=scene.refs.size()) {
        idx = scene.refs.size();
        scene.refs.emplace_back();
    }
    scene.refs[idx] = new ConvertedRootNode;
    scene.refs[idx]->index_in_roots_array = idx;
    return scene.refs[idx];
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
    } else
    {
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
    }
    return true;
}
CoHNode * getNodeByName(const CoHSceneGraph &conv,const QString &a1)
{
    QString filename;
    int idx = a1.lastIndexOf('/');
    if ( idx==-1 )
        filename = a1;
    else
        filename = a1.mid(idx+1);
    return conv.name_to_node.value(filename.toLower(),nullptr);
}
QString  groupMakeName(CoHSceneGraph &conv,const QString &base)
{
    QString buf;
    do
        buf = base + QString::number(++conv.last_node_id);
    while (getNodeByName(conv,buf));
    return buf;
}
bool groupInLibSub(const QString &a1)
{
    if(a1.contains('/')) {
        return !a1.startsWith("maps");
    }
    return !a1.startsWith("grp");
}

// Create new names for any 'numbered' scene nodes
QString groupRename(CoHSceneGraph &conv,NameList &memory, const QString &oldname, bool is_def)
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
    QString tgt = groupMakeName(conv,prefix);
    memory.new_names[querystring] = tgt;
    return tgt;
}
void  groupApplyModifiers(CoHNode *group)
{

    CoHModel *a1 = group->model;
    if ( !a1 )
        return;
    const GeometryModifiers *v4 = findGeomModifier(a1->name, group->dir);
    if ( !v4 )
        return;

    if ( v4->LodNear != 0.0f )
        group->lod_near = v4->LodNear;
    if ( v4->LodFar != 0.0f )
        group->lod_far = v4->LodFar;
    if ( v4->LodNearFade != 0.0f )
        group->lod_near_fade = v4->LodNearFade;
    if ( v4->LodFarFade != 0.0f )
        group->lod_far_fade = v4->LodFarFade;
    if ( v4->LodScale != 0.0f )
        group->lod_scale = v4->LodScale;

    if (v4->LodNear != 0.0f || v4->LodFar != 0.0f || v4->LodNearFade != 0.0f || v4->LodFarFade != 0.0f ||
            v4->LodScale != 0.0f)
        group->lod_fromtrick = true;
}
void setNodeNameAndPath(CoHSceneGraph &scene,CoHNode *a2, QString obj_path)
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
        iter=scene.name_to_node.insert(lowkey,a2);
    }
    a2->name = iter.key();
    a2->dir.clear();
    if ( key.position() != 0 )
        a2->dir = result.mid(0,key.position()-1);
}
void addChildNodes(CoHSceneGraph &conv,const SceneGraphNode_Data &a1, CoHNode *node, NameList &a3)
{
    if ( a1.p_Grp.empty() )
        return;
    node->children.reserve(a1.p_Grp.size());
    for(const GroupLoc_Data & dat : a1.p_Grp)
    {
        QString v5 = groupRename(conv,a3, dat.name, 0);
        NodeChild child;
        child.m_def = getNodeByName(conv,v5);
        if ( !child.m_def )
        {
            groupFileLoadFromName(conv,v5);
            child.m_def = getNodeByName(conv,v5);
        }
        Urho3D::Quaternion quat2 = Quaternion(-dat.rot.x * 180 / M_PI, dat.rot.y * 180 / M_PI, dat.rot.z * 180 / M_PI);
        child.m_matrix.SetRotation(quat2.RotationMatrix());

        child.m_matrix.SetTranslation({dat.pos.x,dat.pos.y,dat.pos.z});
        if ( child.m_def )
            node->children.emplace_back(child);
        else
        {
            qCritical() << "Node"<<node->name<<"\ncan't find member" << dat.name;
        }
    }
}
void addLod(const std::vector<DefLod_Data> &a1, CoHNode *a2)
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
bool nodeCalculateBounds(CoHNode *group)
{
    float geometry_radius=0.0f;
    float maxrad=0.0f;
    CoHModel *model;
    BoundingBox bbox;
    bool set = 0;
    if ( !group )
        return false;
    if ( group->model )
    {
        model = group->model;
        bbox.Merge(toUrho(model->m_min));
        bbox.Merge(toUrho(model->m_max));

        geometry_radius = bbox.size().Length() * 0.5f;
        set = 1;
    }
    for ( NodeChild & child : group->children )
    {
        Vector3 dst(child.m_matrix * Vector3{child.m_def->center.x,child.m_def->center.y,child.m_def->center.z});
        bbox.Merge(dst+Vector3(child.m_def->radius,child.m_def->radius,child.m_def->radius));
        bbox.Merge(dst-Vector3(child.m_def->radius,child.m_def->radius,child.m_def->radius));
        set = 1;
    }
    if ( !set )
    {
        bbox.Clear();
    }
    group->radius = bbox.size().Length() * 0.5f;
    group->m_bbox = bbox;
    group->center = fromUrho(bbox.Center()); // center
    for ( NodeChild & child : group->children )
    {
        glm::vec3 toChildCenter = fromUrho(child.m_matrix * toUrho(child.m_def->center)) - group->center;
        float r = glm::length(toChildCenter) + child.m_def->radius;
        maxrad = std::max(maxrad,r);
    }
    if ( maxrad != 0.0f )
        group->radius = maxrad;
    group->radius = std::max(geometry_radius,group->radius);
    return group->radius == 0.0f && !group->children.empty();
}
void  nodeSetVisBounds(CoHNode *group)
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
        CoHModel *v1 = group->model;
        dv = v1->m_max - v1->m_min;
        maxrad = glm::length(dv) * 0.5f + group->shadow_dist;
        if ( group->lod_far == 0.0f )
        {
            group->lod_far = (maxrad + 10.0f) * 10.0f;
            group->lod_far_fade = group->lod_far * 0.25f;
            group->lod_autogen = 1;
        }
        maxvis = group->lod_far + group->lod_far_fade;
    }
    for (NodeChild &entr : group->children)
    {
        dv = fromUrho(entr.m_matrix * toUrho(entr.m_def->center));
        dv -= group->center;
        maxrad = std::max(maxrad,glm::length(dv) + entr.m_def->radius + entr.m_def->shadow_dist);
        maxvis = std::max(maxvis,glm::length(dv) + entr.m_def->vis_dist * entr.m_def->lod_scale);
    }
    if ( group->shadow_dist == 0.0f )
        group->shadow_dist = maxrad - group->radius;
    group->vis_dist = maxvis;
}
bool addNode(CoHSceneGraph &conv,const SceneGraphNode_Data &defload, NameList &renamer)
{
    if (defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;
    QString obj_path = groupRename(conv,renamer, defload.name, 1);
    CoHNode * a1 = getNodeByName(conv,obj_path);
    if ( !a1 )
        a1 = newDef(conv);
    if ( !defload.p_Obj.isEmpty() )
    {
        a1->model = groupModelFind(defload.p_Obj);
        if ( !a1->model )
        {
            qCritical() << "Cannot find root geometry in" << defload.p_Obj;
        }
        groupApplyModifiers(a1);
    }
    setNodeNameAndPath(conv,a1,obj_path);
    addChildNodes(conv,defload,a1,renamer);

    if ( a1->children.empty() && !a1->model )
    {
        qDebug() << "Should delete def"<<defload.name<<" after conversion it has no children, nor models";
        return false;
    }
    addLod(defload.p_Lod, a1);

    nodeCalculateBounds(a1);
    nodeSetVisBounds(a1);
    return true;
}
void  loadSubgraph(const QString &filename,CoHSceneGraph &conv)
{
    geosetLoad(filename); // load given subgraph's root geoset
    QFileInfo fi(filename);
    loadSceneGraph(conv,fi.path()+"/"+fi.completeBaseName()+".txt");
}
bool groupLoadRequiredLibsForNode(CoHNode *node,CoHSceneGraph &conv)
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
        loadSubgraph(gf->geopath,conv);
    }
    return true;
}
bool groupFileLoadFromName(CoHSceneGraph &conv,const QString &a1)
{
    GeoStoreDef *v3 = groupGetFileEntryPtr(a1);
    if ( !v3 )
        return false;
    if ( v3->loaded )
        return true;

    v3->loaded = true;
    loadSubgraph(v3->geopath,conv);
    groupLoadRequiredLibsForNode(getNodeByName(conv,a1),conv);
    return 1;
}
void addRoot(CoHSceneGraph &conv,const SceneRootNode_Data &refload, NameList &namelist)
{
    QString newname = groupRename(conv,namelist, refload.name, 0);
    auto *def = getNodeByName(conv,newname);
    if(!def) {
        groupFileLoadFromName(conv,newname);
        def = getNodeByName(conv,newname);
    }
    if(!def) {
        qCritical() << "Missing reference:"<<newname;
        return;
    }
    auto ref = newRef(conv);
    ref->node = def;
    rotationFromYPR(ref->mat,{refload.rot.x,refload.rot.y,refload.rot.z});
    ref->mat.SetTranslation(Vector3(&refload.pos[0]));
}
void PostProcessScene(SceneGraph_Data &scenegraph,CoHSceneGraph &conv,NameList &renamer,const QString &name)
{
    for (const SceneGraphNode_Data & node_dat : scenegraph.Def )
        addNode(conv,node_dat, renamer);
    for (const SceneRootNode_Data & root_dat : scenegraph.Ref)
        addRoot(conv,root_dat, renamer);
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
QString mapNameToPath(const QString &a1)
{
    int start_idx = a1.indexOf("object_library",Qt::CaseInsensitive);
    if ( -1==start_idx )
        start_idx = a1.indexOf("maps",Qt::CaseInsensitive);
    QString buf = basepath+"geobin/" + a1.mid(start_idx);
    const int last_dot = buf.lastIndexOf('.');
    if(-1==last_dot)
        buf+=".bin";
    else if(!buf.contains(".crl"))
        buf.replace(last_dot,buf.size()-last_dot,".bin");
    return buf;
}
}
bool loadSceneGraph(CoHSceneGraph &conv,const QString &path)
{
    QString binName = mapNameToPath(path);
    NameList my_name_list;
    my_name_list.basename = buildBaseName(path);
    SceneGraph_Data scenegraph;
    binName.replace("CHUNKS.bin","Chunks.bin");
    LoadSceneData(binName,scenegraph);
    PostProcessScene(scenegraph,conv,my_name_list,path);
    return true;
}
extern int created_node_count;
//TODO: convert this from recursive function into iterative one.
Urho3D::Node * convertedNodeToLutefisk(CoHNode *conv_node, const Urho3D::Matrix3x4 &mat, Context *ctx, int depth, int opt)
{

    ResourceCache* cache = ctx->m_ResourceCache.get();
    Urho3D::Node * node = new Node(ctx);
    created_node_count++;
    node->SetName(conv_node->name);
    node->SetTransform(mat);
    conv_node->m_lutefisk_result = node;
    if(conv_node->model)
    {
        //assert(def->children.empty());
        StaticModel* conv_model = convertedModelToLutefisk(ctx,node,conv_node,opt);
        if(!conv_model)
        {
            delete node;
            return nullptr;
        }
        node->SetVar("CoHNode",conv_node);
        node->SetVar("CoHModel",conv_node->model);
        // some nodes contain both a model and children nodes
        //return node;
    }

    if (depth > 0)
    {
        node->SetVar("CoHNode",conv_node);
        for(NodeChild &d : conv_node->children)
        {
            //this is used to reject models for farther lods
            if(d.m_def->model && d.m_def->lod_near!=0.0f)
                continue;
            Urho3D::Node *newNode = convertedNodeToLutefisk(d.m_def, d.m_matrix, ctx, depth - 1,opt); // recursive call
            if (newNode)
                node->AddChild(newNode);
        }
        return node;
    }
    if (depth == 0)
    { /*
        auto    boxTextNode = node->CreateChild("BoxText");
        Text3D *boxText     = boxTextNode->CreateComponent<Text3D>();
        boxText->SetText(QString("Group %1\n(%2)").arg(conv_node->name).arg(mat.Translation().ToString()));
        boxText->SetFont(cache->GetResource<Font>("Fonts/BlueHighway.sdf"), 18);
        boxText->SetColor(Color::RED);
        boxText->SetFaceCameraMode(FC_ROTATE_Y);*/
    }
    return node;
}



