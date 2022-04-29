/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/anim_definitions.h"
#include "Components/Colors.h"

#include <QtCore/QHash>
#include <QtCore/QString>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

struct TextureAnim_Data;
enum eBlendMode : uint8_t;
enum ModelFlags : uint32_t;

enum TrickFlags : uint32_t
{
    Additive          = 0x1,
    ScrollST0         = 0x2,
    FrontFace         = 0x4,
    CameraFace        = 0x8,
    DistAlpha         = 0x10,
    ColorOnly         = 0x20,
    DoubleSided       = 0x40,
    NoZTest           = 0x80,
    ReflectTex1       = 0x100,
    ScrollST1         = 0x200,
    NightLight        = 0x400,
    NoZWrite          = 0x800,
    Wireframe         = 0x1000,
    NoDraw            = 0x2000,
    STAnimate         = 0x4000,
    ParticleSys       = 0x8000,
    NoColl            = 0x10000,
    SetColor          = 0x20000,
    VertexAlpha       = 0x40000,
    NoFog             = 0x80000,
    FogHasStartAndEnd = 0x100000,
    EditorVisible     = 0x200000,
    CastShadow        = 0x400000,
    LightFace         = 0x800000,
    ReflectTex0       = 0x1000000,
    AlphaRef          = 0x2000000,
    SimpleAlphaSort   = 0x4000000,
    TexBias           = 0x8000000,
    NightGlow         = 0x10000000,
    SelectOnly        = 0x20000000,
    STSScale          = 0x40000000,
    NotSelectable     = 0x80000000,
};

enum class TexOpt
{
    None                = 0,
    Fade                = 1 << 0,
    Truecolor           = 1 << 1,
    MultiTex            = 1 << 3, // synthetic name
    NoRandomAddGlow     = 1 << 4,
    FullBright          = 1 << 5,
    ClampS              = 1 << 6,
    ClampT              = 1 << 7,
    AlwaysAddGlow       = 1 << 8,
    MirrorS             = 1 << 9,
    MirrorT             = 1 << 10,
    Replaceable         = 1 << 11,
    IsBumpMap           = 1 << 12,
    RepeatS             = 1 << 13,
    RepeatT             = 1 << 14,
    Cubemap             = 1 << 15,
    NoMip               = 1 << 16,
    Jpeg                = 1 << 17,
    NoDither            = 1 << 18,
    NoColl              = 1 << 19,
    SurfaceSlick        = 1 << 20,
    SurfaceIcy          = 1 << 21,
    SurfaceBouncy       = 1 << 22,
    Border              = 1 << 23,
    OldTint             = 1 << 24,
    PointSample         = 1 << 26,
    FallbackForceOpaque = 1 << 29
};
inline uint32_t operator &(TexOpt lhs,TexOpt rhs) {
    return (uint32_t)lhs & (uint32_t)rhs;
}
inline TexOpt operator |=(TexOpt lhs,TexOpt rhs) {
    return TexOpt((uint32_t)lhs | (uint32_t)rhs);
}

enum class TextureScrollType
{
    Normal,
    PingPong,
    Oval
};

struct AdvancedTextureEntry
{
    QByteArray        Name;
    glm::vec2         Scale {1,1};
    glm::vec2         Scroll;
    TextureScrollType ScrollType;
    uint8_t           Swappable;
};
struct TextureMaterialGroup
{
    AdvancedTextureEntry Base;
    AdvancedTextureEntry Multiply;
    AdvancedTextureEntry DualColor;
    AdvancedTextureEntry AddGlow;
    AdvancedTextureEntry BumpMap;
};
enum class TextureBlendType
{
    Multiply = 0, // this is confusing, the name Multiply, clashes with value (?MODULATE?)
    ColorBlendDual=2,
    AddGlow=3,
    AlphaDetail=4,
    SunFlare=9
};
struct TextureFallback
{
    glm::vec2            ScaleST0{1, 1};
    glm::vec2            ScaleST1{1, 1};
    QByteArray Base;
    QByteArray Blend;
    QByteArray BumpMap;
    TextureBlendType BlendType;
    uint8_t  UseFallback;
    glm::vec3  DiffuseScaleVec {1,1,1};
    glm::vec3  AmbientScaleVec {1,1,1};
    glm::vec3  AmbientMinVec;
};

struct TextureModifiers
{
    QByteArray           src_file;
    QByteArray           name;
    QByteArray           Blend;
    QByteArray           BumpMap;
    QByteArray           DF_ObjName; // reference to now defunct DoubleFusion in-game advertisement sdk
    glm::vec2            Fade{0, 0};
    glm::vec2            ScaleST0{1, 1};
    glm::vec2            ScaleST1{1, 1};
    uint32_t             TimeStamp;
    TexOpt               Flags;
    ModelFlags           ObjFlags;
    TextureBlendType     BlendType;
    int                  surfaceBitIdx;
    QByteArray           Surface; // Name of this surface  WOOD METAL etc.
    float                Gloss;
    TextureMaterialGroup Layer1;
    AdvancedTextureEntry Mask;
    TextureMaterialGroup Layer2;
    AdvancedTextureEntry CubeMap;

    RGBA                 rgba3;
    RGBA                 rgba4;
    RGBA                 Specular1 = {255,255,255};
    RGBA                 Specular2 = {255,255,255};
    float                SpecularExp1 = 8;
    float                SpecularExp2;
    float Reflectivity=0;
    float ReflectivityBase=-1.0f;
    float ReflectivityScale=-1.0f;
    float ReflectivityPower=-1.0f;
    uint8_t AlphaMask=0;
    float MaskWeight=1;
    uint8_t Multiply1Reflect=0;
    uint8_t Multiply2Reflect=0;
    uint8_t BaseAddGlow=0;
    uint8_t MinAddGlow=0;
    uint8_t MaxAddGlow=128;
    uint8_t AddGlowMat2 = 0;
    uint8_t AddGlowTint = 0;
    uint8_t ReflectionTint = 0;
    uint8_t ReflectionDesaturate = 0;
    uint8_t AlphaWater = 0;
    glm::vec3 DiffuseScaleVec = {1,1,1};
    glm::vec3 AmbientScaleVec = {1,1,1};
    glm::vec3 AmbientMinVec;
    TextureFallback Fallback;
};

struct ColorList
{
    RGBA  field_0[16];
    int   count; // count of color 'key frames'
    float scale;
};

struct ModelModifiers
{
    glm::vec2                 ScrollST0;
    glm::vec2                 ScrollST1;
    glm::vec2                 tex_scale;
    RGBA                      TintColor0;
    RGBA                      TintColor1;
    uint32_t                  _TrickFlags = 0;
    float                     SortBias    = 0;
    struct GeometryModifiers *info        = nullptr;
    bool                      isFlag(TrickFlags v) const { return (_TrickFlags & uint32_t(v)) != 0; }
};

enum GroupFlags : uint32_t
{
    VisOutside       = 0x1,
    VisBlocker       = 0x2,
    VisAngleBlocker  = 0x4,
    VisTray          = 0x8,
    VisShell         = 0x10,
    VisWindow        = 0x20,
    VisDoorFrame     = 0x40,
    RegionMarker     = 0x80,
    VolumeTrigger    = 0x100,
    WaterVolume      = 0x200,
    LavaVolume       = 0x400,
    DoorVolume       = 0x800,
    ParentFade       = 0x1000,
    KeyLight         = 0x2000,
    SewerWaterVolume = 0x4000,
    RedWaterVolume   = 0x8000,  // only in I2
    MaterialVolume   = 0x10000, // only in I2
};
enum LodFlags : uint32_t
{
    ErrorTriCount       = 2,
    UseFallbackMaterial = 0x80
};

struct AutoLOD
{
    float      AllowedError;
    float      LodNearFade;
    float      LodNear;
    float      LodFarFade;
    float      LodFar;
    LodFlags   Lodflags;
    QByteArray ModelName;
};

struct GeometryModifiers
{
    QByteArray                    src_name;
    QByteArray                    name;
    ModelModifiers                node;
    int                           GfxFlags = 0;
    uint32_t                      ObjFlags;
    uint32_t                      GroupFlags;
    eBlendMode                    blend_mode;
    float                         LodNear;
    float                         LodFar;
    float                         LodNearFade;
    float                         LodFarFade;
    std::vector<TextureAnim_Data> StAnim;
    std::vector<AutoLOD>          AutoLOD;
    glm::vec2                     FogDist;
    float                         ShadowDist;
    float                         ObjTexBias;
    float                         CameraFaceTightenUp = 0.0f;
    glm::vec2                     NightGlow;
    glm::vec2                     Sway;
    glm::vec2                     SwayRandomize = {0, 0};
    glm::vec2                     SwayPitch;
    glm::vec2                     SwayRoll;
    float                         AlphaRef;
    float                         WaterReflectionSkew     = 30.0f;
    float                         WaterReflectionStrength = 60.0f;
    ColorList                     clists[2];
    float                         LodScale;
    uint32_t                      m_Timestamp;
};

struct SceneModifiers
{
    std::vector<TextureModifiers>  texture_mods;
    std::vector<GeometryModifiers> geometry_mods;
    // for every directory in the texture's path we can have a modifier.
    QHash<QString, TextureModifiers *>  m_texture_path_to_mod;
    QHash<QString, GeometryModifiers *> g_tricks_string_hash_tab;
};
GeometryModifiers *findGeomModifier(SceneModifiers &tricks, const QString &modelname, const QString &trick_path);
