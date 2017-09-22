#pragma once
#include <glm/vec3.hpp>

#include <QtCore/QString>
#include <vector>

struct GroupLoc_Data
{
  QString name;
  glm::vec3 pos {0,0,0};
  glm::vec3 rot {0,0,0};
};
struct GroupProperty_Data
{
  QString propName;
  QString propValue;
  int propertyType; // 1 - propValue contains float radius, 0 propValue is plain string
};
struct TintColor_Data
{
    uint32_t clr1;
    uint32_t clr2;
};
struct ReplaceTex_Data
{
    int texIdxToReplace;
    QString repl_with;
};

struct DefSound_Data
{
    enum {
        Exclude = 1
    };
    QString name;
    float volRel1;
    float sndRadius;
    float snd_ramp_feet;
    uint32_t sndFlags;
};
struct DefLod_Data
{
    float Far;
    float FarFade;
    float Near;
    float NearFade;
    float Scale;
};
struct DefOmni_Data
{
    uint32_t omniColor;
    float Size;
    int isNegative;
};
struct DefBeacon_Data
{
    QString name;
    float amplitude; // maybe rotation speed ?
};
struct DefFog_Data
{
    float fogZ;
    float fogX;
    float fogY;
    uint32_t fogClr1;
    uint32_t fogClr2;
};
struct DefAmbient_Data
{
    uint32_t clr;
};

struct SceneGraphNode_Data
{
    QString name;
    QString p_Obj;
    QString type;
    int flags;
    std::vector<GroupLoc_Data> p_Grp;
    std::vector<GroupProperty_Data> p_Property;
    std::vector<TintColor_Data> p_TintColor;
    std::vector<DefSound_Data> p_Sound;
    std::vector<ReplaceTex_Data> p_ReplaceTex;
    std::vector<DefOmni_Data> p_Omni;
    std::vector<DefBeacon_Data> p_Beacon;
    std::vector<DefFog_Data> p_Fog;
    std::vector<DefAmbient_Data> p_Ambient;
    std::vector<DefLod_Data> p_Lod;
};
struct SceneRootNode_Data
{
    QString name;
    glm::vec3 pos {0,0,0};
    glm::vec3 rot {0,0,0};
};
struct SceneGraph_Data
{
    int Version;
    std::vector<SceneGraphNode_Data> Def;
    std::vector<SceneRootNode_Data> Ref;
    QString Scenefile;
};
