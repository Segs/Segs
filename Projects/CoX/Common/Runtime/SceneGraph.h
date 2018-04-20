#pragma once
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/common.hpp"
#include "glm/gtc/constants.hpp"

#include <QString>
#include <QHash>
#include <vector>

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
struct Model
{
    QString name;
    AxisAlignedBoundingBox box;
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
    Model *model;
    struct GeoStoreDef *geoset_info; // where is this node from ?
    QString name;
    QString dir;
    AxisAlignedBoundingBox        m_bbox;
    glm::vec3                     center;
    float                         radius        = 0;
    float                         vis_dist      = 0;
    float                         lod_near      = 0;
    float                         lod_far       = 0;
    float                         lod_near_fade = 0;
    float                         lod_far_fade  = 0;
    float                         lod_scale     = 0;
    float                         shadow_dist   = 0;
    bool                          lod_autogen = 0;
    bool                          in_use        = false;
    bool                          lod_fromtrick = false;
};

struct RootNode
{
    glm::mat4x3 mat;
    SceneNode *node = nullptr;
    uint32_t index_in_roots_array=0;
};

struct SceneGraph
{
    std::vector<SceneNode *> all_converted_defs;
    std::vector<RootNode *> refs;
    QHash<QString,SceneNode *> name_to_node;
    QString m_base_path;
    bool loadSceneGraph(const QString &datadir);
    void setBasePath(const QString base_path) {m_base_path=base_path;}

    void loadSubgraph(const QString &filename);
private:
    void serializeIn(struct SceneGraph_Data &scenegraph,struct NameList &renamer);
};

