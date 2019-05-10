/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/AxisAlignedBox.h"

#include <vector>
#include <QStringList>
#include <QHash>

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
    int last_node_id=0; // used to create new number suffixes for generic nodes
    QString m_base_path;
    NameList m_renamer; // used to rename prefab nodes to keep all names unique in the loaded graph
    SceneGraph *m_target;
};
struct PrefabStore
{
    QHash<QString, GeoStoreDef> m_dir_to_geoset;
    QHash<QString, GeoStoreDef *> m_modelname_to_geostore;

    bool prepareGeoLookupArray(const QString &base_path);
    bool loadPrefabForNode(SceneNode *node, LoadingContext &ctx);
    bool loadNamedPrefab(const QString &name, LoadingContext &conv);
    Model *groupModelFind(const QString &path, LoadingContext &ctx);
    GeoStoreDef * groupGetFileEntryPtr(const QString &full_name);
    void sceneGraphWasReset(); // reset 'loaded' flag on all geostores
};

} // namespace SEGS
