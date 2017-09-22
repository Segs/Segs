#include "CoHSceneConverter.h"
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
struct NameList
{
    QHash<QString,QString> new_names; // map from old node name to a new name
    QString basename;
};
static NameList my_name_list;

ConvertedNode *newDef(ConvertedSceneGraph &scene) {
    ConvertedNode *res = new ConvertedNode;
    scene.all_converted_defs.emplace_back(res);
    return res;
}
bool LoadScene(const QString &fname,SceneGraph_Data &scenegraph)
{
    BinStore binfile;

    if(fname.contains(".crl")) {
        if(!loadFrom(fname,scenegraph))
        {
            qCritical() << "Failed to serialize data from crl:" << fname;
            return false;
        }
    }
    else
    {
        if(!binfile.open(fname,scenegraph_i0_2_requiredCrc)) {
            qCritical() << "Failed to open original bin:" << fname;
            return false;
        }
        if(!loadFrom(&binfile,scenegraph)) {
            qCritical() << "Failed to load data from original bin:" << fname;
            return false;
        }
    }
    return true;
}
ConvertedNode * groupDefFind(const ConvertedSceneGraph &conv,const QString &a1)
{
    QString filename;
    int idx = a1.lastIndexOf('/');
    if ( idx==-1 )
        filename = a1;
    else
        filename = a1.mid(idx+1);
    return conv.name_to_def.value(filename.toLower(),nullptr);
}
QString  groupMakeName(ConvertedSceneGraph &conv,const QString &base)
{
    QString buf;
    do
        buf = base + QString::number(++conv.last_node_id);
    while (groupDefFind(conv,buf));
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
QString groupRename(ConvertedSceneGraph &conv,NameList &memory, const QString &oldname, bool is_def)
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
void  groupApplyModifiers(ConvertedNode *group)
{

    ConvertedModel *a1 = group->model;
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
bool addNode(ConvertedSceneGraph &conv,const SceneGraphNode_Data &defload, NameList &renamer,  const QString &filename)
{
    if (defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;
    QString obj_path = groupRename(conv,renamer, defload.name, 1);
    ConvertedNode * a1 = groupDefFind(conv,obj_path);
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
    return true;
}
void addRoot(ConvertedSceneGraph &conv,const SceneRootNode_Data &refload, NameList &namelist)
{
    QString newname = groupRename(conv,namelist, refload.name, 0);
}
void PostProcessScene(SceneGraph_Data &scenegraph,ConvertedSceneGraph &conv,NameList &renamer,const QString &name)
{
    for (const SceneGraphNode_Data & def : scenegraph.Def )
        addNode(conv,def, renamer, name);
    for (const SceneRootNode_Data & ref : scenegraph.Ref)
        addRoot(conv,ref, renamer);
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
    int last_dot = buf.lastIndexOf('.');
    if(-1==last_dot)
        buf+=".bin";
    else
        buf.replace(last_dot,buf.size()-last_dot,".bin");
    return buf;
}
}
bool loadSceneGraph(ConvertedSceneGraph &conv,const QString &path)
{
    QString binName = mapNameToPath(path);
    NameList my_name_list;
    my_name_list.basename = buildBaseName(path);
    SceneGraph_Data scenegraph;
    LoadScene(binName,scenegraph);
    PostProcessScene(scenegraph,conv,my_name_list,path);
    return true;
}




