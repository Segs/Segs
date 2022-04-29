/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/Colors.h"
#include <glm/vec3.hpp>

#include <QtCore/QString>
#include <vector>

enum class GroupNode_Flags
{
    DisablePlanarReflections = 1 << 0
};
struct GroupLoc_Data
{
    QByteArray name;
    glm::vec3 pos {0,0,0};
    glm::vec3 rot {0,0,0};
    GroupNode_Flags flags = GroupNode_Flags(0);
};

struct GroupProperty_Data
{
    QByteArray propName;
    QByteArray propValue;
    int propertyType; // 1 - propValue contains float radius, 0 propValue is plain string
};

struct TintColor_Data
{
    RGBA clr1;
    RGBA clr2;
};

struct ReplaceTex_Data
{
    int texIdxToReplace;
    QByteArray repl_with;
};

struct TexSwap_Data
{
    QByteArray m_TextureName;
    QByteArray m_ReplaceWith;
    int m_Composite;
};
struct DefSound_Data
{
    QByteArray name;
    float volRel1;
    float sndRadius;
    float snd_ramp_feet;
    uint32_t sndFlags; // 1 to exclude
};

struct DefLod_Data
{
    float Far;
    float FarFade;
    float Near=0.0f; // not available in i24
    float NearFade=0.0f; // not available in i24
    float Scale;
};

struct DefOmni_Data
{
    RGBA omniColor;
    float Radius;
    int isNegative;
};

struct Cubemap_Data
{
    int m_GenerateSize = 256;
    int m_CaptureSize = 1024;
    float m_Blur=0.0f;
    float m_Time=12.0f; // time of day ??
};
struct Volume_Data
{
    glm::vec3 m_Scale;
};
struct DefBeacon_Data
{
    QByteArray m_Name;
    float m_Radius;
};

struct DefFog_Data
{
    float m_Radius;
    float m_NearDistance;
    float m_FarDistance;
    RGBA Color1;
    RGBA Color2;
    float m_Speed;
};

struct DefAmbient_Data
{
    RGBA clr;
};

enum class NodeFlags
{
    Ungroupable = 1 << 0,
    FadeNode    = 1 << 1,
    FadeNode2   = 1 << 2,
    NoFogClip   = 1 << 3,
    NoColl      = 1 << 4,
    NoOcclusion = 1 << 5,
};

struct SceneGraphNode_Data
{
    QByteArray m_Name;
    QByteArray m_Object;
    QByteArray m_Type;
    QByteArray m_SoundScript;
    std::vector<GroupLoc_Data> m_Group;
    std::vector<GroupProperty_Data> m_Property;
    std::vector<TintColor_Data> m_TintColor;
    std::vector<DefSound_Data> m_Sound;
    std::vector<ReplaceTex_Data> m_ReplaceTex;
    std::vector<DefOmni_Data> m_Omni;
    std::vector<Cubemap_Data> m_Cubemap;
    std::vector<Volume_Data> m_Volume;
    std::vector<DefBeacon_Data> m_Beacon;
    std::vector<DefFog_Data> m_Fog;
    std::vector<DefAmbient_Data> m_Ambient;
    std::vector<DefLod_Data> m_Lod;
    std::vector<TexSwap_Data> m_TexSwap;
    NodeFlags m_Flags = NodeFlags(0);
    float m_Alpha;
};

struct SceneRootNode_Data
{
    QByteArray name;
    glm::vec3 pos {0,0,0};
    glm::vec3 rot {0,0,0};
};

struct SceneGraph_Data
{
    std::vector<SceneGraphNode_Data> Def;
    std::vector<SceneRootNode_Data> Ref;
    QByteArray Scenefile;
    QByteArray LoadScreen; // i24
    std::vector<QByteArray> Import; //i24
    int Version;
};
