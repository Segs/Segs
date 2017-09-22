#include "CoHSceneConverter.h"
#include "CohModelConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"

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
AllTricks_Data g_tricks_store;
QHash<QString,GeometryModifiers *> tricks_string_hash_tab;
struct GeoStoreDef
{
    QString geopath;
    QStringList entries;
    bool loaded;
};
struct NameList
{
    QHash<QString,QString> new_names; // map from old node name to a new name
    QString basename;
};
static QHash<QString,GeoStoreDef> m_dir_to_geoset;
static QHash<QString,GeoStoreDef *> m_def_to_geo;
static NameList my_name_list;

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
bool addNode(ConvertedSceneGraph &conv,const SceneGraphNode_Data &defload, NameList &renamer,  const QString &filename)
{
    if (defload.p_Grp.empty() && defload.p_Obj.isEmpty())
        return false;
    QString obj_path = groupRename(conv,renamer, defload.name, 1);
    ConvertedNode * a1 = groupDefFind(conv,obj_path);
    if ( !a1 )
        a1 = newDef(conv);
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
static void setupTrick(GeometryModifiers *a1)
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
    auto iter = tricks_string_hash_tab.find(a1->name.toLower());
    if (iter!=tricks_string_hash_tab.end())
    {
        qDebug() << "duplicate model trick!";
        return;
    }
    tricks_string_hash_tab[a1->name.toLower()]=a1;
}

static void  trickLoadPostProcess(int a1, AllTricks_Data *a2)
{
    g_texture_path_to_mod.clear();
    tricks_string_hash_tab.clear();
    for (TextureModifiers &texopt : a2->texture_mods)
        setupTexOpt(&texopt);
    for (GeometryModifiers &trickinfo : a2->geometry_mods)
        setupTrick(&trickinfo);
}
bool loadTricksBin()
{
    BinStore binfile;
    QString fname(basepath+"bin/tricks.bin");
    {
        if (!(binfile.open(fname, tricks_i0_requiredCrc) || binfile.open(fname, tricks_i2_requiredCrc)))
        {
            qCritical() << "Failed to open original bin:" << fname;
            return false;
        }
        if (!loadFrom(&binfile, &g_tricks_store))
        {
            qCritical() << "Failed to open data from original bin:" << fname;
            return false;
        }
    }
    trickLoadPostProcess(0,&g_tricks_store);
    return true;

}
bool prepareGeoLookupArray()
{
    QFile defnames(basepath+"bin/defnames.bin");
    if(!defnames.open(QFile::ReadOnly))
    {
        qCritical() << "Failed to open defnames.bin";
        return false;
    }
    QByteArrayList defnames_arr;
    for(const QByteArray &entr : defnames.readAll().split('\0'))
        defnames_arr.push_back(entr);
    QString lookup_str;
    QString geo_path;
    GeoStoreDef *current_geosetinf;
    for (QString str : defnames_arr )
    {
        str.replace("CHUNKS.geo","Chunks.geo");
        int last_slash = str.lastIndexOf('/');
        if(-1!=last_slash)
        {
            geo_path = str.mid(0,last_slash);
            lookup_str = geo_path.toLower();
            current_geosetinf = &m_dir_to_geoset[lookup_str];
            current_geosetinf->geopath = geo_path;
        }
        current_geosetinf->entries << str.mid(last_slash + 1);
        m_def_to_geo[str.mid(last_slash + 1)] = current_geosetinf;
    }

    return loadTricksBin();
}

