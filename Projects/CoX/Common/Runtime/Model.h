/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/AxisAlignedBox.h"
#include "Common/Runtime/Handle.h"
#include "Common/Runtime/HandleBasedStorage.h"

#include <QString>

#include <vector>
#include <memory>
#include <functional>

struct ModelModifiers;
struct TextureModifiers;
class QFile;

template <int idx_bits, int gen_bits, typename T>
struct SingularStoreHandleT;

namespace SEGS
{
struct GeoSet;
enum class CoHBlendMode : uint8_t;
using HTexture = SingularStoreHandleT<20,12,struct TextureWrapper>;

struct VBOPointers
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> uv1;
    std::vector<glm::vec2> uv2;
    std::vector<glm::ivec3> triangles;
    std::vector<HTexture> assigned_textures;
    bool needs_tangents=false;
};
enum ModelFlags : uint32_t
{
    OBJ_ALPHASORT      = 0x1,
    OBJ_FULLBRIGHT     = 0x4,
    OBJ_NOLIGHTANGLE   = 0x10,
    OBJ_DUALTEXTURE    = 0x40,
    OBJ_LOD            = 0x80,
    OBJ_TREE           = 0x100,
    OBJ_DUALTEX_NORMAL = 0x200,
    OBJ_FORCEOPAQUE    = 0x400,
    OBJ_BUMPMAP        = 0x800,
    OBJ_WORLDFX        = 0x1000,
    OBJ_CUBEMAP        = 0x2000,
    OBJ_DRAW_AS_ENT    = 0x4000,
    OBJ_STATICFX       = 0x8000,
    OBJ_HIDE           = 0x10000,
};

#pragma pack(push, 1)
struct DeltaPack
{
    int compressed_size=0;
    uint32_t uncomp_size=0;
    int buffer_offset=0;
    uint8_t *compressed_data=nullptr;
};

struct TextureBind
{
    uint16_t tex_idx;
    uint16_t tri_count;
};

#pragma pack(pop)
struct PackBlock
{
    DeltaPack  tris;
    DeltaPack  verts;
    DeltaPack  norms;
    DeltaPack  sts;
    DeltaPack  weights;
    DeltaPack  matidxs;
    DeltaPack  grid;
    DeltaPack &operator[](uint8_t idx) { return (&tris)[idx]; }
};

struct Model
{
    AxisAlignedBoundingBox   box;
    QString                  name;
    int                      flags;
    float                    visibility_radius;
    uint32_t                 num_textures;
    PackBlock                packed_data;
    std::vector<TextureBind> texture_bind_info;
    ptrdiff_t                boneinfo_offset = 0;
    GeoSet *                 geoset;
    ModelModifiers *         trck_node = nullptr;
    glm::vec3                scale;
    uint32_t                 vertex_count;
    uint32_t                 model_tri_count;
    CoHBlendMode             blend_mode;
    std::unique_ptr<VBOPointers> vbo;
};
void geosetLoadHeader(QFile &fp, GeoSet *geoset);
void geosetLoadData(QFile &fp, GeoSet *geoset);
void initLoadedModel(std::function<HTexture (const QString &)> funcloader,Model *model,const std::vector<HTexture> &textures);
void fillVBO(Model & model);
} // namespace SEGS
