/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/serialization_common.h"

#include <QStringList>
#include <QHash>
#include <QSet>

#include <vector>


namespace SEGS
{
struct NodeLoadRequest;
struct Model;
struct SceneGraph;
struct SceneNode;

struct GeoSet
{
    QByteArray           geopath;
    QByteArray           name;
    std::vector<Model *> subs;
    std::vector<QByteArray> tex_names;
    std::vector<char>    m_geo_data;
    uint32_t             geo_data_size;
    bool                 data_loaded = false;
};
Model *getModelById(GeoSet *gset, int id);

// Geo file info
struct GeoStoreDef
{
    QByteArray geopath;        //!< a path to a .geo file
    QByteArrayList entries;    //!< the names of models contained in a geoset
    bool loaded;
};
struct NameList
{
    QHash<QByteArray, QByteArray> new_names; // map from old node name to a new name
    QByteArray basename;
};

struct LoadingContext
{
    LoadingContext(int depth) : m_nesting_level(depth) {}
    NameList m_renamer; // used to rename prefab nodes to keep all names unique in the loaded graph
    QByteArray m_base_path;
    SceneGraph* m_target;
    FSWrapper *fs_wrap;
    int last_node_id=0; // used to create new number suffixes for generic nodes
    int m_nesting_level=0; // how deep are we in include hierarchy
    bool prevent_nesting=false;

};
struct PrefabStore
{
    QHash<QByteArray, GeoStoreDef> m_dir_to_geoset;
    QHash<QByteArray, GeoStoreDef *> m_modelname_to_geostore;
    QSet<QByteArray> m_missing_geosets;
    FSWrapper *m_fs;
    QByteArray m_base_path;

    PrefabStore(FSWrapper* fs,const QByteArray &bp) : m_fs(fs),m_base_path(bp) {}

    bool prepareGeoLookupArray(const QString &base_path);
    bool loadPrefabForNode(SceneNode *node, LoadingContext &ctx);
    bool loadNamedPrefab(const QByteArray &name, LoadingContext &conv, NodeLoadRequest *load_request=nullptr);
    Model *groupModelFind(const QByteArray &path, LoadingContext &ctx);
    Model *modelFind(const QString &geoset_name, const QString &model_name, LoadingContext &ctx);
    GeoStoreDef * groupGetFileEntryPtr(const QByteArray &full_name);
    void sceneGraphWasReset(); // reset 'loaded' flag on all geostores
};

} // namespace SEGS
