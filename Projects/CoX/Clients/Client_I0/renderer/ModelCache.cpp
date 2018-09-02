#include "ModelCache.h"

#include "utils/helpers.h"
#include "Model.h"

#include <GL/glew.h>
#include "RendererState.h"
#include <utils/dll_patcher.h>
#include "RendererUtils.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"

#include <cstring>
#include <cstdio>
#include "entity/EntClient.h"
#include "RenderTricks.h"
#include "3rd_party/mikktspace/mikktspace.h"
#include <unordered_set>
struct BoneInfo;
extern "C"
{
    __declspec(dllimport) void anim_Unpack_SASR_Buffer(SmallAnimSRel *pack, char *unpacked_data, int entry_size, size_t num_entries, int type);
    __declspec(dllimport) void anim_Unpack_SASR(SmallAnimSRel *pack, void *dest);
    __declspec(dllimport) void model_cache_4D3D90(Model *model); //modelFixup
    __declspec(dllimport) Parser_Trick *tricks_FindTrick(const char *bonename, char *trick_path);

    __declspec(dllimport) RdrStats struct_9E7300;
    __declspec(dllimport) int int32_9ED15C; //total_vbos
}
struct MikktUserDataWrapper
{

    GeometryData *m_geometry;
    int count_tris=0;
    MikktUserDataWrapper(Model *m) : m_geometry(m->vbo),count_tris(m->model_tri_count){
    }

    // Returns the number of faces (triangles/quads) on the mesh to be processed.
    static int getNumFaces(const SMikkTSpaceContext * pContext)
    {
        MikktUserDataWrapper *self = (MikktUserDataWrapper *)pContext->m_pUserData;
        return self->count_tris;
    }

    // Returns the number of vertices on face number iFace
    // iFace is a number in the range {0, 1, ..., getNumFaces()-1}
    static int getNumVerticesOfFace(const SMikkTSpaceContext * /*pContext*/, const int /*iFace*/)
    {
        return 3;
    }

    static void getPosition(const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
    {
        MikktUserDataWrapper *self = (MikktUserDataWrapper *)pContext->m_pUserData;
        memcpy(fvPosOut, self->m_geometry->vertices[self->m_geometry->triangles[iFace][iVert]].data(), sizeof(Vector3));
    }
    static void getNormal(const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert)
    {
        MikktUserDataWrapper *self = (MikktUserDataWrapper *)pContext->m_pUserData;
        memcpy(fvNormOut,self->m_geometry->normals_offset[self->m_geometry->triangles[iFace][iVert]].data(),sizeof(Vector3));
    }
    static void getTexCoord(const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
    {
        MikktUserDataWrapper *self = (MikktUserDataWrapper *)pContext->m_pUserData;
        memcpy(fvTexcOut, self->m_geometry->uv1_offset[self->m_geometry->triangles[iFace][iVert]].data(),
               sizeof(Vector2));
    }
    static void setTSpaceBasic(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
    {
        MikktUserDataWrapper *self = (MikktUserDataWrapper *)pContext->m_pUserData;
        assert(iFace < self->count_tris);
        int vert_idx = self->m_geometry->triangles[iFace][iVert];
        Vector4 &tgt(self->m_geometry->tangents_directions[vert_idx]);
        memcpy(tgt.data(),fvTangent,sizeof(Vector3));
        tgt.w = fSign;
    }
};
template<class T>
static T* getMemoryLocationAndIncrementOffset(char *base_mem,size_t vert_count,size_t &offset)
{
    T* res= reinterpret_cast<T *>(base_mem + offset);
    offset += sizeof(T) * vert_count;
    return res;
}
static void CalculateTangentArray(long vertexCount, const glm::vec3 *vertex, const glm::vec3 *normal,
    const glm::vec2 *texcoord, long triangleCount, const glm::ivec3 *triangle, glm::vec4 *tangent)
{
    glm::vec3 *tan1 = new glm::vec3[vertexCount * 2];
    glm::vec3 *tan2 = tan1 + vertexCount;
    memset(tan1, 0,vertexCount * sizeof(glm::vec3) * 2);

    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->x;
        long i2 = triangle->y;
        long i3 = triangle->z;

        const glm::vec3& v1 = vertex[i1];
        const glm::vec3& v2 = vertex[i2];
        const glm::vec3& v3 = vertex[i3];

        const glm::vec2& w1 = texcoord[i1];
        const glm::vec2& w2 = texcoord[i2];
        const glm::vec2& w3 = texcoord[i3];

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float r = 1.0F / (s1 * t2 - s2 * t1);
        glm::vec3 sdir{ (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r };
        glm::vec3 tdir{ (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r };

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;

        triangle++;
    }

    for (long a = 0; a < vertexCount; a++)
    {
        const glm::vec3& n = normal[a];
        const glm::vec3& t = tan1[a];

        // Gram-Schmidt orthogonalize
        const glm::vec3 value = normalize(t - n * dot(n, t));
        // Calculate handedness
        float direction = (dot(cross(n, t), tan2[a]) < 0.0F) ? -1.0f : 1.0f;
        tangent[a] = glm::vec4(value, direction);
    }

    delete[] tan1;
}

void processWithMikt(Model *m)
{
    static SMikkTSpaceInterface access_interface;
    static SMikkTSpaceContext wrap;
    MikktUserDataWrapper val(m);
    access_interface.m_getNumFaces = MikktUserDataWrapper::getNumFaces;
    access_interface.m_getNumVerticesOfFace = MikktUserDataWrapper::getNumVerticesOfFace;
    access_interface.m_getPosition = MikktUserDataWrapper::getPosition;
    access_interface.m_getNormal = MikktUserDataWrapper::getNormal;
    access_interface.m_getTexCoord = MikktUserDataWrapper::getTexCoord;
    access_interface.m_setTSpaceBasic = MikktUserDataWrapper::setTSpaceBasic;
    access_interface.m_setTSpace = nullptr;
    wrap.m_pInterface = &access_interface;
    wrap.m_pUserData = &val;
    genTangSpaceDefault(&wrap);
    wrap.m_pUserData = nullptr;
}
static void tangentBasis(const glm::vec3 &pv0, const  glm::vec3 &pv1, const  glm::vec3 &pv2, const  glm::vec2 &t0, const  glm::vec2 &t1, const  glm::vec2 &t2, glm::vec3 &n, glm::vec3 &tangent, glm::vec3 &bitangent)
{
    using namespace glm;
    vec2 dUV1 = t1 - t0;
    vec2 dUV2 = t2 - t0;
    vec3 v13a = cross((pv2 - pv0),n);
    vec3 v20a = cross((pv1 - pv0),n);
    float fac1 = dUV2.y * dUV1.x - dUV2.x * dUV1.y;
    if (std::abs(fac1) > 0.00001f)
    {
        float r = 1.0f / fac1;
        tangent = (v13a*dUV2.y - v20a * dUV1.y) * r; // Tangent
        bitangent = (v20a*dUV1.x - v13a * dUV2.x) * r; // Bitangent
    }
    tangent = normalize(tangent);
    bitangent = normalize(bitangent);
}
static void bump_Process(Model *model)
{
    if (!(model->Model_flg1 & 0x4800))
        return;
    assert(model->model_tri_count > 0);
//    original_BumpProcess(model);
    //processWithMikt(model);
    GeometryData *modelvbo = model->vbo;
    assert(modelvbo->normals_offset);
    assert(modelvbo->uv1_offset);
    glm::vec3 *pos = (glm::vec3 *)modelvbo->cpuside_memory;
    glm::vec3 *normals = (glm::vec3 *)modelvbo->normals_offset;
    glm::vec2 *uv = (glm::vec2 *)modelvbo->uv1_offset;
    glm::ivec3 *triangles = (glm::ivec3 *)modelvbo->triangles;
    glm::vec4 *packed_tangents = (glm::vec4 *)modelvbo->tangents_directions;
    CalculateTangentArray(model->vertex_count, pos, normals, uv, model->model_tri_count, triangles, packed_tangents);

}
static bool hasBumpMap(Model *model)
{
    return (model->Model_flg1 & 0x4800) != 0;
}

void segs_modelSetupVertexObject(Model *model, int useVbos)
{
    size_t positionsTotalSize;
    char *mem;
    int total_size;
    size_t size;
    char buff[128] = { 0 };
    size_t bumpTotalSize=0;
    GeometryData *vbo = model->vbo;

    assert(vbo);
    if (vbo->vertices || vbo->gl_index_buffer)
        return;
    GLDebugGuard guard(__FUNCTION__);

    size_t boneIndicesTotalSize = 0;
    size_t weightsTotalSize = 0;
    size_t num_tex_coords2 = 0;
    size_t num_tex_coords1 = 0;
    size_t normalsTotalSize = 0;

    size_t model_vertex_count = size_t(model->vertex_count);
    if (!model->boneinfo)
        model->boneinfo = assignDummyBoneInfo(model->bone_name_offset);
    size                = sizeof(Vector3i) * model->model_tri_count;
    vbo->triangles      = (Vector3i *)COH_MALLOC(size);
    anim_Unpack_SASR_Buffer(model->sasr, (char *)vbo->triangles, 3, model->model_tri_count, 1);
    positionsTotalSize = sizeof(Vector3) * model_vertex_count;
    if (model->sasr[2].uncomp_size)
        normalsTotalSize = sizeof(Vector3) * model_vertex_count;
    if (model->sasr[3].uncomp_size)
    {
        num_tex_coords1 = sizeof(Vector2) * model_vertex_count;
        num_tex_coords2 = sizeof(Vector2) * model_vertex_count;
    }
    const int hasBump = hasBumpMap(model);
    if (hasBump)
    {
        bumpTotalSize = sizeof(Vector4) * model_vertex_count;// *2;
    }
    if (model->Model_flg1 & 0x4000)
    {
        weightsTotalSize = 8 * model_vertex_count;
        boneIndicesTotalSize  = 4 * model_vertex_count;
    }
    total_size    = normalsTotalSize + num_tex_coords1 + num_tex_coords2 + bumpTotalSize + weightsTotalSize + boneIndicesTotalSize + positionsTotalSize;
    mem           = (char *)COH_CALLOC(total_size, 1);
    vbo->cpuside_memory = (float *)mem;

    size_t offset = 0;
    vbo->vertices = getMemoryLocationAndIncrementOffset<Vector3>(mem,model_vertex_count,offset);
    anim_Unpack_SASR_Buffer(&model->sasr[1], (char *)vbo->vertices, 3, model_vertex_count, 0);
    if (normalsTotalSize)
    {
        vbo->normals_offset = getMemoryLocationAndIncrementOffset<Vector3>(mem,model_vertex_count,offset);
        anim_Unpack_SASR_Buffer(&model->sasr[2], (char *)vbo->normals_offset, 3, model_vertex_count, 0);
    }
    if (num_tex_coords1)
    {
        vbo->uv1_offset = getMemoryLocationAndIncrementOffset<Vector2>(mem,model_vertex_count,offset);
        anim_Unpack_SASR_Buffer(&model->sasr[3], (char *)vbo->uv1_offset, 2, model_vertex_count, 0);
    }
    if (num_tex_coords2)
    {
        vbo->uv2_offset = getMemoryLocationAndIncrementOffset<Vector2>(mem,model_vertex_count,offset);
        memcpy(vbo->uv2_offset, vbo->uv1_offset, num_tex_coords1);
    }
    if (weightsTotalSize || boneIndicesTotalSize)
    {
        vbo->weights = getMemoryLocationAndIncrementOffset<Vector2>(mem,model_vertex_count,offset);
        vbo->boneIndices = getMemoryLocationAndIncrementOffset<Vector2si>(mem,model_vertex_count,offset);
        if (model->sasr[5].uncomp_size)
        {
            uint8_t *weight_buf = (uint8_t *)malloc(model_vertex_count);
            uint8_t *index_buf = (uint8_t *)malloc(2 * model_vertex_count);
            anim_Unpack_SASR(&model->sasr[4], (int *)weight_buf);
            anim_Unpack_SASR(&model->sasr[5], (int *)index_buf);
            for (size_t i = 0; i < model_vertex_count; ++i)
            {
                vbo->boneIndices[i].i0 = index_buf[2 * i];
                vbo->boneIndices[i].i1 = index_buf[2 * i + 1];
                vbo->weights[i].x      = weight_buf[i] / 255.0f;
                vbo->weights[i].y      = 1.0f - vbo->weights[i].x;
            }
            free(weight_buf);
            free(index_buf);
        }
        else
        {
            for (size_t j = 0; j < model_vertex_count; ++j)
            {
                vbo->boneIndices[j].i0 = 0;
                vbo->boneIndices[j].i1 = 0;
                vbo->weights[j].x      = 1.0;
                vbo->weights[j].y      = 0.0;
            }
        }
    }
    if (bumpTotalSize)
    {
        vbo->tangents_directions = getMemoryLocationAndIncrementOffset<Vector4>(mem, model_vertex_count, offset);
        //vbo->binormals = getMemoryLocationAndIncrementOffset<Vector3>(mem,model_vertex_count,offset);
        bump_Process(model);
    }
    model_cache_4D3D90(model);
    if (useVbos == 2)
        return;

    vbo->createVAO();
    glBindVertexArray(vbo->segs_data->vao_id);
    const char *last_part = strrchr(model->parent_anim->name, '/') + 1;

    glGenBuffers(1, &vbo->gl_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->gl_index_buffer);
    snprintf(buff, 128, "%s_IBO_%d",last_part,size);
    glObjectLabel(GL_BUFFER, vbo->gl_index_buffer, -1, buff);
    if (size)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, vbo->triangles, GL_STATIC_DRAW);
        FreeMem(vbo->triangles, 1);
        vbo->triangles = nullptr;
    }
    int32_9ED15C += total_size + size;
    glGenBuffers(1, &vbo->gl_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->gl_vertex_buffer);
    snprintf(buff, 128, "%s_VBO_%d",last_part,total_size);
    glObjectLabel(GL_BUFFER, vbo->gl_vertex_buffer, -1, buff);
    glBufferData(GL_ARRAY_BUFFER, total_size, mem, GL_STATIC_DRAW);


    if (vbo->normals_offset)
        vbo->normals_offset = (Vector3 *)((char *)vbo->normals_offset - (char *)vbo->vertices);
    if (vbo->uv1_offset)
        vbo->uv1_offset = (Vector2 *)((char *)vbo->uv1_offset - (char *)vbo->vertices);
    if (vbo->uv2_offset)
        vbo->uv2_offset = (Vector2 *)((char *)vbo->uv2_offset - (char *)vbo->vertices);
    if (vbo->tangents_directions)
        vbo->tangents_directions = (Vector4 *)((char *)vbo->tangents_directions - (char *)vbo->vertices);
    if (vbo->weights)
        vbo->weights = (Vector2 *)((char *)vbo->weights - (char *)vbo->vertices);
    if (vbo->boneIndices)
        vbo->boneIndices = (Vector2si *)((char *)vbo->boneIndices - (char *)vbo->vertices);
    vbo->vertices        = nullptr;
    glBindVertexArray(0);
    struct_9E7300.vert_access_count += model->vertex_count;
}
void segs_addModelStubs(AnimList *anm)
{
    ModelHeader *header = anm->headers;
    assert(header);
    for(int idx_sub = 0; idx_sub<header->num_subs; ++idx_sub)
    {
        Model *model = &header->models[idx_sub];
        model->vbo = new GeometryData;
        Parser_Trick *instance_mods = tricks_FindTrick(model->bone_name_offset, nullptr);
        if (instance_mods)
        {
            if (!model->trck_node)
                model->trck_node = (TrickNode *)COH_MALLOC(sizeof(TrickNode));
            *model->trck_node  =instance_mods->node;
            model->trck_node->info = instance_mods;
        }
    }
}
void patch_modelcache()
{
    PATCH_FUNC(modelSetupVertexObject);
    PATCH_FUNC(addModelStubs);
}
