#include "Prefab.h"

#include "SceneGraph.h"
#include "Model.h"
#include "Common/GameData/trick_serializers.h"
#include "Common/GameData/GameDataStore.h"
#include "Common/GameData/scenegraph_serializers.h" //for getFilepathCaseInsensitive

#include <QDir>
#include <QDebug>

namespace SEGS
{
struct GeoSet;
}

QHash<QString,SEGS::GeoSet *> s_name_to_geoset;

using namespace SEGS;

GeoSet *findAndPrepareGeoSet(FSWrapper &fs,const QString &fname,const QString &base_path)
{
    GeoSet *geoset = nullptr;
    QString name_fixed = fname;
    name_fixed.replace(".anm", ".geo");
    QString true_path = getFilepathCaseInsensitive(fs,base_path + name_fixed);

    QIODevice *fp = fs.open(true_path);
    if(fp)
    {
        geoset = new GeoSet;
        //TODO: QDir(base_path).relativeFilePath(true_path) should be provided by fs service.
        geoset->geopath = QDir(base_path).relativeFilePath(true_path).toUtf8();
        geosetLoadHeader(fp, geoset);
        fp->seek(0);
        s_name_to_geoset[fname] = geoset;
        delete fp;
    }
    else
        qCritical() << "Can't find .geo file" << fname;

    return geoset;
}

/// load the given geoset, used when loading scene-subgraph and nodes
GeoSet * geosetLoad(FSWrapper &fs, const QString &m, const QString &base_path)
{
    GeoSet * res = s_name_to_geoset.value(m,nullptr);
    if(res)
        return res;

    return findAndPrepareGeoSet(fs,m,base_path);
}

Model *PrefabStore::modelFind(const QString &geoset_name, const QString &model_name, LoadingContext &ctx)
{
    Model *ptr_sub = nullptr;
    if(model_name.isEmpty() || geoset_name.isEmpty())
    {
        qCritical() << "Bad model/geometry set requested:";
        if(!model_name.isEmpty())
            qCritical() << "Model: " << model_name;
        if(!geoset_name.isEmpty())
            qCritical() << "GeoFile: " << geoset_name;
        return nullptr;
    }

    GeoSet *geoset = geosetLoad(*ctx.fs_wrap,geoset_name, m_base_path);
    if(!geoset) { // failed to load the geometry set
        m_missing_geosets.insert(geoset_name.toUtf8());
        return nullptr;
    }

    int end_of_name_idx = model_name.indexOf("__");
    if(end_of_name_idx == -1)
        end_of_name_idx = model_name.size();

    QStringRef basename(model_name.midRef(0, end_of_name_idx));

    for(Model *m : geoset->subs)
    {
        QString geo_name = m->name;
        if(geo_name.isEmpty())
            continue;

        bool subs_in_place = (geo_name.size() <= end_of_name_idx || geo_name.midRef(end_of_name_idx).startsWith("__"));
        if(subs_in_place && geo_name.startsWith(basename, Qt::CaseInsensitive))
            ptr_sub = m; // TODO: return immediately
    }

    return ptr_sub;
}

bool PrefabStore::prepareGeoLookupArray(const QString &base_path)
{
    QFile defnames(base_path + "bin/defnames.bin");
    if(!defnames.open(QFile::ReadOnly))
    {
        qCritical() << "Failed to open bin/defnames.bin";
        return false;
    }

    QByteArray lookup_str;
    GeoStoreDef *current_geosetinf = nullptr;
    QByteArrayList defnames_arr = defnames.readAll().replace("CHUNKS.geo", "Chunks.geo").split('\0');
    for(QByteArray str : defnames_arr)
    {
        int last_slash = str.lastIndexOf('/');
        if(-1 != last_slash)
        {
            QByteArray geo_path = str.mid(0, last_slash);
            lookup_str = geo_path.toLower();
            current_geosetinf = &m_dir_to_geoset[lookup_str];
            current_geosetinf->geopath = geo_path;
        }
        current_geosetinf->entries << str.mid(last_slash + 1);
        m_modelname_to_geostore[str.mid(last_slash + 1)] = current_geosetinf;
    }

    return true;
}

bool PrefabStore::loadPrefabForNode(SceneNode *node, LoadingContext &ctx) //groupLoadRequiredLibsForNode
{
    GeoStoreDef *gf;

    if(!node || !node->in_use)
        return false;

    if(node->m_geoset_info)
        gf = node->m_geoset_info;
    else
    {
        gf = groupGetFileEntryPtr(node->m_name);
        node->m_geoset_info = gf;
        if(!node->m_geoset_info)
            node->m_geoset_info = (GeoStoreDef *)-1; // prevent future load attempts
    }

    if(!gf || gf == (GeoStoreDef *)-1)
        return false;

    if(!gf->loaded)
    {
        gf->loaded = true;
        geosetLoad(*ctx.fs_wrap,gf->geopath, m_base_path); // load given subgraph's root geoset
        loadSubgraph(gf->geopath,ctx,*this);
    }

    return true;
}
bool PrefabStore::loadNamedPrefab(const QByteArray &name, LoadingContext &ctx, NodeLoadRequest* load_request) //groupFileLoadFromName
{
    GeoStoreDef *geo_store = groupGetFileEntryPtr(name);
    if(!geo_store)
        return false;
    if(ctx.prevent_nesting)
    {
        if(load_request)
        {
            QFileInfo geofi(geo_store->geopath);
            QString base_file = geofi.path();
            load_request->base_file = base_file.toUtf8();
            load_request->node_name = qPrintable(QFileInfo(name).fileName());
            assert(geo_store->entries.contains(load_request->node_name));
        }
    }
    if(geo_store->loaded)
        return true;

    geo_store->loaded = true;
    if (ctx.prevent_nesting)
        return true;
    // load given prefab's geoset
    GeoSet *gs = geosetLoad(*ctx.fs_wrap,geo_store->geopath, m_base_path);
    if(!gs) {

    }
    loadSubgraph(geo_store->geopath,ctx,*this);
    return loadPrefabForNode(getNodeByName(*ctx.m_target,name), ctx);
}

Model *PrefabStore::groupModelFind(const QByteArray &path, LoadingContext &ctx)
{
    QByteArray model_name = path.mid(path.lastIndexOf('/') + 1);
    auto val = groupGetFileEntryPtr(model_name);
    return val ? modelFind(val->geopath, model_name,ctx) : nullptr;
}

GeoStoreDef * PrefabStore::groupGetFileEntryPtr(const QByteArray &full_name)
{
    QByteArray key = full_name.mid(full_name.lastIndexOf('/') + 1);
    key = key.mid(0, key.indexOf("__"));
    return m_modelname_to_geostore.value(key, nullptr);
}

void PrefabStore::sceneGraphWasReset()
{
    for(auto & v : m_dir_to_geoset)
        v.loaded = false;
}

Model *getModelById(GeoSet *gset, int id)
{
    for (Model *v : gset->subs)
    {
        if (id == v->m_id)
            return v;
    }
    return nullptr;
}
