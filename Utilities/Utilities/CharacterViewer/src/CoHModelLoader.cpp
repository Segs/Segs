/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "CoHModelLoader.h"

#include "CohModelConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "Lutefisk3D/Graphics/Tangent.h"

#include <QFile>
#include <QDebug>
#include <QDir>
extern QString basepath;

using namespace Urho3D;

// convenience struct for triangle index access

// Start of anonymous namespace
namespace {
glm::vec3 fromUrho(Vector3 v) {return {v.x_,v.y_,v.z_};}
Vector3 toUrho(glm::vec3 v) {return {v.x,v.y,v.z};}
} // end of anonymus namespace

float *combineBuffers(SEGS::VBOPointers &meshdata,SEGS::Model *mdl)
{
    size_t num_floats = mdl->vertex_count*3;
    size_t offset = 3*sizeof(float);
    size_t normal_offset;
    size_t texcoord_offset;
    size_t tangents_offset;
    if(!meshdata.norm.empty())
    {
        normal_offset = offset;
        num_floats += mdl->vertex_count*3;
        offset += 3*sizeof(float);
    }
    if(!meshdata.uv1.empty())
    {
        texcoord_offset = offset;
        num_floats += mdl->vertex_count*2;
        offset += 2*sizeof(float);
    }
    if(!meshdata.uv2.empty())
    {
        num_floats += mdl->vertex_count*2;
        offset += 2*sizeof(float);
    }
    if (meshdata.needs_tangents)
    {
        tangents_offset = offset;
        num_floats += mdl->vertex_count * 4;
        offset += 4*sizeof(float);
    }
    if(mdl->hasBoneWeights())
    {
        // 4 floats for weights, 1 int for indices
        num_floats += mdl->vertex_count * 5;
        offset += 5*sizeof(float);
    }
    float *res = new float[num_floats];
    int off=0;
    bool has_weights=mdl->hasBoneWeights();
    for(uint32_t i=0; i<mdl->vertex_count; ++i) {
        res[off++] = meshdata.pos[i].x;
        res[off++] = meshdata.pos[i].y;
        res[off++] = meshdata.pos[i].z;
        if(!meshdata.norm.empty())
        {
            res[off++] = meshdata.norm[i].x;
            res[off++] = meshdata.norm[i].y;
            res[off++] = meshdata.norm[i].z;
        }
        if(!meshdata.uv1.empty())
        {
            res[off++] = meshdata.uv1[i].x;
            res[off++] = meshdata.uv1[i].y;
        }
        if(!meshdata.uv2.empty())
        {
            res[off++] = meshdata.uv2[i].x;
            res[off++] = meshdata.uv2[i].y;
        }
        if (meshdata.needs_tangents)
        {
            off+=4; // memory will be filled in GenerateTangents
        }
        if(has_weights)
        {
            res[off++] = meshdata.bone_weights[i].x;
            res[off++] = meshdata.bone_weights[i].y;
            res[off++] = 0;
            res[off++] = 0;
            uint8_t *res_u8 = (uint8_t *)res;
            res_u8[off] = meshdata.bone_indices[i].first;
            res_u8[off+1] = meshdata.bone_indices[i].second;
            res_u8[off+2] = res_u8[off+3] = 0;
            off++;
        }
    }
    if (meshdata.needs_tangents)
        GenerateTangents(res, offset, meshdata.triangles.data(), sizeof(uint32_t), 0,
                         3 * meshdata.triangles.size(), normal_offset, texcoord_offset, tangents_offset);
    return res;
}


//! @}
