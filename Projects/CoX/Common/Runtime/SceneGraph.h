#pragma once
#include "GameData/scenegraph_definitions.h"

#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/common.hpp"
#include "glm/gtc/constants.hpp"

#include <QString>
#include <QHash>
#include <vector>

struct LoadingContext;

struct AxisAlignedBoundingBox
{
    void merge(glm::vec3 pt)
    {
        m_min = glm::min(m_min,pt);
        m_max = glm::max(m_max,pt);
    }
    void merge(AxisAlignedBoundingBox box)
    {
        m_min = glm::min(m_min,box.m_min);
        m_max = glm::max(m_max,box.m_max);
    }
    void clear()
    {
        m_min = m_max = {0,0,0};
    }
    glm::vec3 size() const { return m_max - m_min; }
    glm::vec3 center()const { return (m_max+m_min)/0.5f;}
    glm::vec3 m_min;
    glm::vec3 m_max;
};
struct NameList
{
    QHash<QString, QString> new_names; // map from old node name to a new name
    QString basename;
};

struct Model
{
    QString name;
    int                      flags;
    float                    visibility_radius;
    uint32_t                 num_textures;
    glm::vec3                scale;
    AxisAlignedBoundingBox   box;
};
struct SceneNodeChildTransform
{
    struct SceneNode *node;
    glm::mat3x3 m_matrix2;
    glm::vec3 m_translation;
};
struct SceneNode
{
    struct GeoStoreDef *    belongs_to_geoset = nullptr;
    std::vector<SceneNodeChildTransform> children;
    std::vector<GroupProperty_Data> *properties = nullptr;

    Model *model = nullptr;
    struct GeoStoreDef *geoset_info = nullptr; // where is this node from ?
    QString name;
    QString dir;
    AxisAlignedBoundingBox        m_bbox;
    int                           m_index_in_scenegraph=0;
    glm::vec3                     center;
    float                         radius        = 0;
    float                         vis_dist      = 0;
    float                         lod_near      = 0;
    float                         lod_far       = 0;
    float                         lod_near_fade = 0;
    float                         lod_far_fade  = 0;
    float                         lod_scale     = 0;
    float                         shadow_dist   = 0;
    bool                          lod_autogen   = false;
    bool                          in_use        = false;
    bool                          lod_fromtrick = false;
};

struct RootNode
{
    glm::mat4 mat;
    SceneNode *node = nullptr;
    uint32_t index_in_roots_array=0;
};

struct SceneGraph
{
    std::vector<SceneNode *> all_converted_defs;
    std::vector<RootNode *> refs;

    QHash<QString,SceneNode *> name_to_node;
};
// Geo file info
struct GeoStoreDef
{
    QString geopath;        //!< a path to a .geo file
    QStringList entries;    //!< the names of models contained in a geoset
    bool loaded;
};
struct PrefabStore
{
    QHash<QString, GeoStoreDef> m_dir_to_geoset;
    QHash<QString, GeoStoreDef *> m_modelname_to_geostore;

    bool prepareGeoLookupArray(const QString &base_path);
    bool loadPrefabForNode(SceneNode *node, LoadingContext &ctx);
    bool loadNamedPrefab(const QString &name, LoadingContext &conv);
    Model *groupModelFind(const QString &path, LoadingContext &ctx);
    GeoStoreDef * groupGetFileEntryPtr(const QString &a1);
};

struct LoadingContext
{
    int last_node_id=0; // used to create new number suffixes for generic nodes
    QString m_base_path;
    NameList m_renamer; // used to rename prefab nodes to keep all names unique in the loaded graph
    SceneGraph *m_target;
};

bool loadSceneGraph(const QString &path, LoadingContext &ctx, PrefabStore &prefabs);
