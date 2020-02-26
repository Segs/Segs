/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/AxisAlignedBox.h"

#include <vector>
#include <QStringList>
#include <QHash>

#include "serialization_common.h"

namespace SEGS
{
struct Model;
struct SceneGraph;
struct SceneNode;

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
Model *getModelById(GeoSet *gset, int id);

// Geo file info
struct GeoStoreDef
{
    QString geopath;        //!< a path to a .geo file
    QStringList entries;    //!< the names of models contained in a geoset
    bool loaded;
};
struct NameList
{
    QHash<QString, QString> new_names; // map from old node name to a new name
    QString basename;
};

struct LoadingContext
{
    LoadingContext(int depth) : m_nesting_level(depth) {}
    FSWrapper *fs_wrap;
    int last_node_id=0; // used to create new number suffixes for generic nodes
    QString m_base_path;
    NameList m_renamer; // used to rename prefab nodes to keep all names unique in the loaded graph
    SceneGraph *m_target;
    int m_nesting_level=0; // how deep are we in include hierarchy
};
struct PrefabStore
{
    QHash<QString, GeoStoreDef> m_dir_to_geoset;
    QHash<QString, GeoStoreDef *> m_modelname_to_geostore;
    FSWrapper *m_fs;
    QString m_base_path;

    PrefabStore(FSWrapper* fs,const QString &bp) : m_fs(fs),m_base_path(bp) {}

    bool prepareGeoLookupArray(const QString &base_path);
    bool loadPrefabForNode(SceneNode *node, LoadingContext &ctx);
    bool loadNamedPrefab(const QString &name, LoadingContext &conv);
    Model *groupModelFind(const QString &path, LoadingContext &ctx);
    Model *modelFind(const QString &geoset_name, const QString &model_name, LoadingContext &ctx);
    GeoStoreDef * groupGetFileEntryPtr(const QString &full_name);
    void sceneGraphWasReset(); // reset 'loaded' flag on all geostores
};

} // namespace SEGS
