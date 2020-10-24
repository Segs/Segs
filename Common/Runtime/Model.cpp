#include "Model.h"
#include "Prefab.h"
#include "RuntimeData.h"
#include "Texture.h"

#include "Common/GameData/trick_definitions.h"
#include "Common/GameData/GameDataStore.h"
#include "Common/GameData/trick_definitions.h"

#include <QStringList>
#include <QHash>
#include <QDebug>
#include <QFile>

using namespace SEGS;

namespace
{
enum UnpackMode
{
    UNPACK_FLOATS=0,
    UNPACK_INTS=1,
};

inline QByteArray uncompr_zip(char *comp_data,int size_comprs,uint32_t size_uncom)
{
    QByteArray compressed_data;
    compressed_data.reserve(size_comprs+4);
    compressed_data.append( char((size_uncom >> 24) & 0xFF));
    compressed_data.append( char((size_uncom >> 16) & 0xFF));
    compressed_data.append( char((size_uncom >> 8) & 0xFF));
    compressed_data.append( char((size_uncom >> 0) & 0xFF));
    compressed_data.append(comp_data,size_comprs);
    return qUncompress(compressed_data);
}

struct GeosetHeader32
{
    char name[128];
    int  unkn1;
    int  subs_idx;
    int  num_subs;
};
// name of the geometry is constructed from actual name and an optional modifer name
struct TexBlockInfo
{
    uint32_t size1;
    uint32_t texname_blocksize;
    uint32_t bone_names_size;
    uint32_t tex_binds_size;
};


struct ModelBones_32
{
    int cnt;
    int bone_ID[15];
    int float_pairs;
    int short_pairs;
};
static_assert(sizeof(ModelBones_32) == 0x48, "sizeof(ModelBones_32)==0x48");
struct PackInfo
{
    int      compressed_size;
    uint32_t uncomp_size;
    int      compressed_data_off;
};
static_assert(sizeof(PackInfo) == 12, "sizeof(PackInfo)==12");

struct Model32
{
    uint32_t        flg1;
    float           radius;
    int             vbo;
    uint32_t        num_textures;
    int16_t         id;
    char            blend_mode;
    char            loadstate;
    int             boneinfo;
    int             trck_node;
    uint32_t        vertex_count;
    uint32_t        model_tri_count;
    int             texture_bind_offsets;
    int             unpacked_1;
    glm::vec3       grid_pos;
    float           grid_size;
    float           grid_invsize;
    float           grid_tag;
    int             grid_numbits;
    int             ctris;
    int             triangle_tags;
    int             bone_name_offset;
    int             num_altpivots;
    int             extra;
    glm::vec3       m_scale;
    glm::vec3       m_min;
    glm::vec3       m_max;
    int             geoset_list_idx;
    PackInfo        pack_data[7];
};
ptrdiff_t unpackDeltaPack(int *tgt_buf, uint8_t *data, uint32_t entry_size, uint32_t num_entries, UnpackMode v_type)
{
    uint32_t     idx;
    float        float_acc[3] = {0, 0, 0};
    int          int_acc[3]   = {0, 0, 0};
    float *      float_tgt;
    int          bit_offset;
    uint8_t *    data_src;
    int          processed_val = 0;
    int *        delta_flags;
    unsigned int extracted_2bits_;
    float        scaling_val;
    uint32_t     entry_idx;
    float        inv_scale;

    bit_offset  = 0;
    scaling_val = 1.0;
    float_tgt   = (float *)tgt_buf;
    delta_flags = (int *)data;
    data_src    = &data[(entry_size * 2 * num_entries + 7) / 8];
    inv_scale   = (float)(1 << *data_src);
    data_src++;
    if(inv_scale != 0.0f)
        scaling_val = 1.0f / inv_scale;
    for (entry_idx = 0; entry_idx < num_entries; ++entry_idx)
    {
        for (idx = 0; idx < entry_size; ++idx)
        {
            extracted_2bits_ = ((unsigned int)delta_flags[bit_offset >> 5] >> (bit_offset & 0x1F)) & 3;
            bit_offset += 2;
            switch (extracted_2bits_)
            {
            case 0: processed_val = 0; break;
            case 1: processed_val = *data_src++ - 127; break;
            case 2:
                processed_val = ((data_src[1] << 8) | *data_src) - 32767;
                data_src += 2;
                break;
            case 3:
                processed_val = (data_src[3] << 24) | (data_src[2] << 16) | (data_src[1] << 8) | *data_src;
                data_src += 4;
                break;
            default: break;
            }
            switch (v_type)
            {
            case UNPACK_FLOATS:
            {
                float extracted_val;
                if(extracted_2bits_ == 3)
                    extracted_val = *(float *)&processed_val;
                else
                    extracted_val = (float)processed_val * scaling_val;
                float_acc[idx] += extracted_val;
                *float_tgt = float_acc[idx];
                ++float_tgt;
                break;
            }
            case UNPACK_INTS:
            {
                int_acc[idx] += processed_val + 1;
                *tgt_buf = int_acc[idx];
                ++tgt_buf;
                break;
            }
            }
        }
    }
    return data_src - data;
}

void geoUnpackDeltas(const DeltaPack *src, uint8_t *target, uint32_t entry_size, uint32_t num_entries, UnpackMode type)
{
    if(0 == src->uncomp_size)
        return;
    ptrdiff_t consumed_bytes;
    if(src->compressed_size)
    {
        QByteArray unpacked = uncompr_zip((char *)src->compressed_data, src->compressed_size, src->uncomp_size);
        consumed_bytes = unpackDeltaPack((int *)target, (uint8_t *)unpacked.data(), entry_size, num_entries, type);
    }
    else
    {
        consumed_bytes = unpackDeltaPack((int *)target, (uint8_t *)src->compressed_data, entry_size, num_entries, type);
    }
}

inline void geoUnpackDeltas(const DeltaPack *src, glm::vec3 *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(src, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_FLOATS);
}

inline void geoUnpackDeltas(const DeltaPack *src, int *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(src, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_INTS);
}
inline void geoUnpack(const DeltaPack *src, char *dest)
{
    if (!src->uncomp_size)
        return;

    if (src->compressed_size)
    {
        assert(src->uncomp_size != 0);
        QByteArray unc = uncompr_zip((char *)src->compressed_data, src->compressed_size, src->uncomp_size);
        memcpy(dest, unc.data(), unc.size());
    }
    else
        memcpy(dest, src->compressed_data, src->uncomp_size);
}
void fixupDataPtr(DeltaPack &a, uint8_t *b)
{
    if(a.uncomp_size)
        a.compressed_data = b + a.buffer_offset;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end of anonymous namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace SEGS
{

void  addModelStubs(GeoSet *geoset)
{
    RuntimeData &rd(getRuntimeData());
    for(Model * m : geoset->subs)
    {
        GeometryModifiers *gmod = findGeomModifier(*rd.m_modifiers, m->name, QString());
        if(gmod)
        {
            if(!m->trck_node)
                m->trck_node = new ModelModifiers;
            *m->trck_node = gmod->node;
            m->trck_node->info = gmod;
        }
    }
}
static std::vector<TextureBind> convertTexBinds(int cnt, const uint8_t *data)
{
    std::vector<TextureBind> res;
    res.assign((const TextureBind *)data,((const TextureBind *)data)+cnt);
    return res;
}
static Model *convertAndInsertModel(GeoSet &tgt, const Model32 *v)
{
    Model *z = new Model;

    z->flags = v->flg1;
    z->visibility_radius = v->radius;
    z->num_textures = v->num_textures;
    z->m_id = v->id;
    z->boneinfo_offset = v->boneinfo;
    z->blend_mode = CoHBlendMode(v->blend_mode);
    z->vertex_count = v->vertex_count;
    z->model_tri_count = v->model_tri_count;
    z->scale = v->m_scale;
    z->box.m_min = v->m_min;
    z->box.m_max = v->m_max;
    for(uint8_t i = 0; i < 7; ++i)
    {
        DeltaPack &dp_blk(z->packed_data[i]);
        dp_blk.compressed_size = v->pack_data[i].compressed_size;
        dp_blk.uncomp_size = v->pack_data[i].uncomp_size;
        dp_blk.compressed_data = nullptr;
        dp_blk.buffer_offset = v->pack_data[i].compressed_data_off;
    }
    tgt.subs.push_back(z);
    return z;
}

static void convertTextureNames(const int *a1, std::vector<QByteArray> &a2)
{
    int   num_textures          = a1[0];
    const int * indices         = a1 + 1;
    const char *start_of_strings_area = (const char *)a1 + num_textures * 4 + sizeof(int);
    a2.reserve(num_textures);
    for(int idx = 0; idx < num_textures; ++idx)
    {
        // fixup the offsets by adding the end of index area
        a2.emplace_back(start_of_strings_area + indices[idx]);
    }
}

void geosetLoadHeader(QIODevice *fp, GeoSet *geoset)
{
    unsigned int anm_hdr_size;
    const uint8_t * stream_pos_0;
    const uint8_t * stream_pos_1;
    uint32_t headersize;
    fp->read((char *)&anm_hdr_size, 4u);
    anm_hdr_size -= 4;
    fp->read((char *)&headersize, sizeof(uint32_t));

    QByteArray zipmem = fp->read(anm_hdr_size);
    QByteArray unc_arr = uncompr_zip(zipmem.data(), anm_hdr_size, headersize);

    const uint8_t * mem = (const uint8_t *)unc_arr.data();

    const TexBlockInfo *info = (const TexBlockInfo *)mem;
    geoset->geo_data_size = info->size1;

    convertTextureNames((const int *)(mem + sizeof(TexBlockInfo)), geoset->tex_names);
    stream_pos_0            = mem + info->texname_blocksize + sizeof(TexBlockInfo);
    stream_pos_1            = stream_pos_0 + info->bone_names_size;
    const GeosetHeader32 *header32  = (const GeosetHeader32 *)(stream_pos_1 + info->tex_binds_size);
    const Model32 *     ptr_subs  = (Model32 *)(stream_pos_1 + info->tex_binds_size + sizeof(GeosetHeader32));
    geoset->name = header32->name;
    bool has_alt_pivot=false;
    for(int idx = 0; idx < header32->num_subs; ++idx)
    {
        const Model32 *sub_model = &ptr_subs[idx];
        std::vector<TextureBind> binds;
        if(info->tex_binds_size)
            binds = convertTexBinds(sub_model->num_textures, sub_model->texture_bind_offsets + stream_pos_1);

        if (sub_model->num_altpivots > 0)
            has_alt_pivot |= true;
        Model *m    = convertAndInsertModel(*geoset, sub_model);
        m->texture_bind_info = binds;
        m->geoset       = geoset;
        m->name         = QByteArray((const char *)stream_pos_0 + sub_model->bone_name_offset);
    }

    if(!geoset->subs.empty())
        addModelStubs(geoset);
    if (has_alt_pivot)
        qDebug() << "Alternate model pivots were not converted";
}

void modelFixup(const Model &model,VBOPointers &vbo)
{
    if(!vbo.norm.empty() && (model.flags & OBJ_NOLIGHTANGLE))
    {
        for(uint32_t i = 0; i<model.vertex_count; ++i)
            vbo.norm[i] = glm::vec3(1, -1, 1);
    }

    if( vbo.uv1.empty())
        return;

    bool texture_scaling_used = false;
    for(uint32_t i = 0; i < model.vertex_count; ++i )
        vbo.uv1[i].y = 1.0f - vbo.uv1[i].y;

    if(!vbo.uv2.empty())
    {
        for(uint32_t i = 0; i < model.vertex_count; ++i)
            vbo.uv2[i].y = 1.0f - vbo.uv2[i].y;
    }

//    for(HTexture tex : vbo.assigned_textures)
//    {
//        if(!tex->info)
//            continue;

//        if( 1.0f != tex->scaleUV0.x || 1.0f != tex->scaleUV0.y ||
//             1.0f != tex->scaleUV1.x || 1.0f != tex->scaleUV1.y )
//            texture_scaling_used = true;
//    }

//    if(!texture_scaling_used)
//        return;

//    std::vector<bool> vertex_uv_was_scaled(model.vertex_count);
//    uint32_t triangle_offset = 0;
//    for(uint32_t j = 0; j < model.num_textures; ++j )
//    {
//        TextureWrapper &tex(vbo.assigned_textures[j].get());
//        const uint32_t bind_tri_count = model.texture_bind_info[j].tri_count;
//        if(!tex.info)
//            continue;

//        glm::vec2 scaletex0 = tex.scaleUV0;
//        glm::vec2 scaletex1 = tex.scaleUV1;
//        for(uint32_t tri_idx = 0; tri_idx < bind_tri_count; ++tri_idx)
//        {
//            int *tri(vbo.triangles.data()+(tri_idx+triangle_offset)*3);
//            for(int vnum=0; vnum<3; ++vnum)
//            {
//                const uint32_t vert_idx = tri[vnum];
//                if(vertex_uv_was_scaled[vert_idx])
//                    continue;

//                vertex_uv_was_scaled[vert_idx] = true;
//                vbo.uv2[vert_idx].x *= scaletex0.x;
//                vbo.uv2[vert_idx].y *= scaletex0.y;
//                vbo.uv1[vert_idx].x *= scaletex1.x;
//                vbo.uv1[vert_idx].y *= scaletex1.y;
//            }
//        }
//        triangle_offset += bind_tri_count;
//    }




}

static bool bumpMapped(const Model &model)
{
    return model.flags & (OBJ_DRAW_AS_ENT | OBJ_BUMPMAP);
}

std::unique_ptr<VBOPointers> fillVbo(const Model &model)
{
    std::unique_ptr<VBOPointers> vbo = std::make_unique<VBOPointers>();
    std::vector<int> &triangles(vbo->triangles);
    triangles.resize(model.model_tri_count*3);//, 1, ".\\render\\model_cache.c", 138);
    geoUnpackDeltas(&model.packed_data.tris, triangles.data(), model.model_tri_count);
    uint32_t total_size = 0;
    uint32_t Vertices3D_bytes = sizeof(glm::vec3) * model.vertex_count;
    total_size += Vertices3D_bytes;
    if(model.packed_data.norms.uncomp_size)
        total_size += Vertices3D_bytes;
    if( model.packed_data.sts.uncomp_size )
        total_size += 2*sizeof(glm::vec2) * model.vertex_count;

    vbo->pos.resize(model.vertex_count);

    geoUnpackDeltas(&model.packed_data.verts, vbo->pos.data(), model.vertex_count);
    if(model.packed_data.norms.uncomp_size)
    {
        vbo->norm.resize(model.vertex_count);
        geoUnpackDeltas(&model.packed_data.norms, vbo->norm.data(), model.vertex_count);
    }

    if(model.packed_data.sts.uncomp_size)
    {
        vbo->uv1.resize(model.vertex_count);
        geoUnpackDeltas(&model.packed_data.sts, (uint8_t *)vbo->uv1.data(), 2, model.vertex_count, UNPACK_FLOATS);
        vbo->uv2 = vbo->uv1;
    }
    if(model.hasBoneWeights())
    {
        std::vector<uint8_t> weights;
        std::vector<std::pair<uint8_t,uint8_t>> indices;
        weights.resize(model.vertex_count);
        indices.resize(model.vertex_count);

        vbo->bone_weights.resize(model.vertex_count);
        vbo->bone_indices.resize(model.vertex_count);

        geoUnpack(&model.packed_data.weights, (char *)weights.data());
        geoUnpack(&model.packed_data.matidxs, (char *)indices.data());
        for(uint32_t i=0; i<model.vertex_count; ++i)
        {
            vbo->bone_indices[i] = std::make_pair(uint16_t(indices[i].first),uint16_t(indices[i].second));
            vbo->bone_weights[i].x = (weights[i]/255.0f);
            vbo->bone_weights[i].y = 1.0f - vbo->bone_weights[i].x;
        }
    }
    if(bumpMapped(model))
        vbo->needs_tangents = true;

    return vbo;
}

void fillVBO(Model & model)
{
    std::unique_ptr<VBOPointers> databuf(fillVbo(model));
    modelFixup(model,*databuf);
    model.vbo = std::move(databuf);
}
static void convertModelBones(Model *m, ModelBones_32 *src)
{
    m->bone_info_data = new BoneInfo;
    BoneInfo *tgt = m->bone_info_data;
    tgt->numbones = src->cnt;
    assert(src->cnt < 15);
    for (int i = 0; i < src->cnt; ++i)
        tgt->bone_ID[i] = src->bone_ID[i];
}
void geosetLoadData(QIODevice *fp, GeoSet *geoset)
{
    int buffer;
    fp->seek(0);
    fp->read((char *)&buffer, 4);
    fp->seek(buffer + 8);
    geoset->m_geo_data.resize(geoset->geo_data_size); //, 1, "\\src\\Common\\seq\\anim.c", 496);
    fp->read(geoset->m_geo_data.data(), geoset->geo_data_size);
    uint8_t *buffer_b = (uint8_t *)geoset->m_geo_data.data();

    for(Model *current_sub : geoset->subs)
    {
        fixupDataPtr(current_sub->packed_data.tris, buffer_b);
        fixupDataPtr(current_sub->packed_data.verts, buffer_b);
        fixupDataPtr(current_sub->packed_data.norms, buffer_b);
        fixupDataPtr(current_sub->packed_data.sts, buffer_b);
        fixupDataPtr(current_sub->packed_data.grid, buffer_b);
        fixupDataPtr(current_sub->packed_data.weights, buffer_b);
        fixupDataPtr(current_sub->packed_data.matidxs, buffer_b);
        if(current_sub->boneinfo_offset)
        {
            convertModelBones(current_sub, (ModelBones_32 *)(buffer_b + current_sub->boneinfo_offset));
        }
    }
    geoset->data_loaded = true;
}


void toSafeModelName(char *inp, int cnt)
{
    for(int i=0; i<cnt; ++i)
        if(inp[i]=='?')
            inp[i] = '^';
}

} // end SEGS namespace
