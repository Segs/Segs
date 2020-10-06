/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/Colors.h"
#include "Common/GameData/anim_definitions.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <vector>


struct TextureAnim_Data;
enum eBlendMode : uint8_t;

enum TrickFlags  : uint32_t
{
    Additive    =           0x1,
    ScrollST0   =           0x2,
    FrontFace   =           0x4,
    CameraFace  =           0x8,
    DistAlpha   =          0x10,
    ColorOnly   =          0x20,
    DoubleSided =          0x40,
    NoZTest     =          0x80,
    ReflectTex1 =         0x100,
    ScrollST1   =         0x200,
    NightLight  =         0x400,
    NoZWrite    =         0x800,
    Wireframe   =        0x1000,
    NoDraw      =        0x2000,
    STAnimate   =        0x4000,
    ParticleSys =        0x8000,
    NoColl      =       0x10000,
    SetColor    =       0x20000,
    VertexAlpha =       0x40000,
    NoFog       =       0x80000,
    FogHasStartAndEnd= 0x100000,
    EditorVisible =    0x200000,
    CastShadow  =      0x400000,
    LightFace   =      0x800000,
    ReflectTex0 =     0x1000000,
    AlphaRef    =     0x2000000,
    SimpleAlphaSort = 0x4000000,
    TexBias     =     0x8000000,
    NightGlow   =    0x10000000,
    SelectOnly  =    0x20000000,
    STSScale    =    0x40000000,
    NotSelectable =  0x80000000,
};

struct TextureModifiers
{
    QByteArray   src_file;
    QByteArray   name;
    QByteArray   Blend;
    QByteArray   BumpMap;
    glm::vec2 Fade{0, 0};
    glm::vec2 ScaleST0{0, 0};
    glm::vec2 ScaleST1{0, 0};
    uint32_t  Flags;
    uint32_t  BlendType;
    int       surfaceBitIdx;
    QByteArray  Surface; // Name of this surface  WOOD METAL etc.
    float     Gloss;
};

struct ColorList
{
    RGBA field_0[16];
    int count; // count of color 'key frames'
    float scale;
};

struct ModelModifiers
{
    glm::vec2 ScrollST0;
    glm::vec2 ScrollST1;
    glm::vec2 tex_scale;
    RGBA TintColor0;
    RGBA TintColor1;
    uint32_t _TrickFlags=0;
    float SortBias=0;
    struct GeometryModifiers *info=nullptr;
    bool isFlag(TrickFlags v) const {
        return (_TrickFlags & uint32_t(v))!=0;
    }
};

enum GroupFlags : uint32_t
{
    VisOutside       =    0x1,
    VisBlocker       =    0x2,
    VisAngleBlocker  =    0x4,
    VisTray          =    0x8,
    VisShell         =   0x10,
    VisWindow        =   0x20,
    VisDoorFrame     =   0x40,
    RegionMarker     =   0x80,
    VolumeTrigger    =  0x100,
    WaterVolume      =  0x200,
    LavaVolume       =  0x400,
    DoorVolume       =  0x800,
    ParentFade       = 0x1000,
    KeyLight         = 0x2000,
    SewerWaterVolume = 0x4000,
    RedWaterVolume   = 0x8000, // only in I2
    MaterialVolume   =0x10000, // only in I2
};

struct GeometryModifiers
{
    QByteArray                    src_name;
    QByteArray                    name;
    ModelModifiers                node;
    int                           GfxFlags;
    uint32_t                      ObjFlags;
    uint32_t                      GroupFlags;
    eBlendMode                    blend_mode;
    float                         LodNear;
    float                         LodFar;
    float                         LodNearFade;
    float                         LodFarFade;
    std::vector<TextureAnim_Data> StAnim;
    glm::vec2                     FogDist;
    float                         ShadowDist;
    float                         AlphaRef;
    float                         ObjTexBias;
    glm::vec2                     NightGlow;
    float                         Sway;
    float                         Sway_Rotate;
    ColorList                     clists[2];
    float                         LodScale;
};

enum TexOpt : uint32_t
{
    FADE        = 0x0001,
    DUAL        = 0x0010,
    REPLACEABLE = 0x0800,
    BUMPMAP     = 0x1000,
};

struct SceneModifiers
{
    std::vector<TextureModifiers>  texture_mods;
    std::vector<GeometryModifiers> geometry_mods;
    // for every directory in the texture's path we can have a modifier.
    QHash<QString,TextureModifiers *> m_texture_path_to_mod;
    QHash<QString,GeometryModifiers *> g_tricks_string_hash_tab;
};
GeometryModifiers *findGeomModifier(SceneModifiers &tricks,const QString &modelname, const QString &trick_path);
