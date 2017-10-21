#include "CohModelConverter.h"
#include "CoHSceneConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"

#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Graphics/Material.h>
#include <Lutefisk3D/Graphics/Technique.h>
#include <Lutefisk3D/Graphics/Model.h>
#include <Lutefisk3D/Graphics/Geometry.h>
#include <Lutefisk3D/Graphics/VertexBuffer.h>
#include <Lutefisk3D/Graphics/IndexBuffer.h>
#include <Lutefisk3D/IO/File.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/Vector3.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <QFile>
#include <QDebug>
#include <QDir>
extern QString basepath;

using namespace Urho3D;

// convenience struct for triangle index access
struct Vector3i
{
    uint32_t xi,yi,zi;
    uint32_t operator[](int idx) const { return ((const uint32_t *)this)[idx];}
};
struct VBOPointers
{
    Vector3 *pos=nullptr;
    Vector3 *norm=nullptr;
    Vector2 *uv1=nullptr;
    Vector2 *uv2=nullptr;
    std::vector<Vector3i> triangles;
    std::vector<TextureWrapper> assigned_textures;
};

// Start of anonymous namespace
namespace {
enum UnpackMode {
    UNPACK_FLOATS=0,
    UNPACK_INTS=1,
};
// name of the geometry is constructed from actual name and an optional modifer name
struct TexBlockInfo
{
    uint32_t size1;
    uint32_t texname_blocksize;
    uint32_t bone_names_size;
    uint32_t tex_binds_size;
};
struct GeosetHeader32
{
    char name[124];
    int  parent_idx;
    int  unkn1;
    int  subs_idx;
    int  num_subs;
};
struct PackInfo
{
    int      compressed_size;
    uint32_t uncomp_size;
    int      compressed_data_off;
};
static_assert(sizeof(PackInfo) == 12, "sizeof(PackInfo)==12");
struct Model32
{
    int             flg1;
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
    Vector3         grid_pos;
    float           grid_size;
    float           grid_invsize;
    float           grid_tag;
    int             grid_numbits;
    int             ctris;
    int             triangle_tags;
    int             bone_name_offset;
    int             num_altpivots;
    int             extra;
    Vector3         m_scale;
    Vector3         m_min;
    Vector3         m_max;
    int             geoset_list_idx;
    PackInfo        pack_data[7];
};

AllTricks_Data s_tricks_store;
/// this map is used too lookup converted models by the ConvertedModel pointer
std::unordered_map<ConvertedModel *,Urho3D::SharedPtr<Model>> s_coh_model_to_renderable;
QHash<QString,GeoStoreDef> s_dir_to_geoset;
QHash<QString,GeoStoreDef *> s_modelname_to_geostore;
QHash<QString,GeometryModifiers *> tricks_string_hash_tab;
QHash<QString,ConvertedGeoSet *> s_name_to_geoset;

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

static void setupTexOpt(TextureModifiers *tex)
{
    if (tex->ScaleST0.x == 0.0f)
        tex->ScaleST0.x = 1.0f;
    if (tex->ScaleST0.y == 0.0f)
        tex->ScaleST0.y = 1.0f;
    if (tex->ScaleST1.x == 0.0f)
        tex->ScaleST1.x = 1.0f;
    if (tex->ScaleST1.y == 0.0f)
        tex->ScaleST1.y = 1.0f;
    if (tex->Fade.x != 0.0f || tex->Fade.y != 0.0f)
        tex->Flags |= uint32_t(TexOpt::FADE);
    if (!tex->Blend.isEmpty())
        tex->Flags |= uint32_t(TexOpt::DUAL);
    if (!tex->Surface.isEmpty())
    {
        //qDebug() <<"Has surface"<<tex->Surface;
    }

    tex->name = tex->name.mid(0,tex->name.lastIndexOf('.')); // cut last extension part
    if(tex->name.startsWith('/'))
        tex->name.remove(0,1);
    auto iter = g_texture_path_to_mod.find(tex->name.toLower());
    if (iter!=g_texture_path_to_mod.end())
    {
        qDebug() << "duplicate texture info: "<<tex->name;
        return;
    }
    g_texture_path_to_mod[tex->name.toLower()] = tex;
}
static void setupTrick(GeometryModifiers *a1)
{
    if (a1->node.TintColor0.rgb_are_zero())
        a1->node.TintColor0 = RGBA(0xFFFFFFFF);
    if (a1->node.TintColor1.rgb_are_zero())
        a1->node.TintColor1 = RGBA(0xFFFFFFFF);
    a1->AlphaRef /= 255.0f;
    if (a1->ObjTexBias != 0.0f)
        a1->node._TrickFlags |= TexBias;
    if (a1->AlphaRef != 0.0f)
        a1->node._TrickFlags |= AlphaRef;
    if (a1->FogDist.x != 0.0f || a1->FogDist.y != 0.0f)
        a1->node._TrickFlags |= FogHasStartAndEnd;
    if (a1->ShadowDist != 0.0f)
        a1->node._TrickFlags |= CastShadow;
    if (a1->NightGlow.x != 0.0f || a1->NightGlow.y != 0.0f)
        a1->node._TrickFlags |= NightGlow;
    if (a1->node.ScrollST0.x != 0.0f || a1->node.ScrollST0.y != 0.0f)
        a1->node._TrickFlags |= ScrollST0;
    if (a1->node.ScrollST1.x != 0.0f || a1->node.ScrollST1.y != 0.0f)
        a1->node._TrickFlags |= ScrollST1;
    if (!a1->StAnim.empty())
    {
        //        if (setStAnim(&a1->StAnim.front()))
        //            a1->node._TrickFlags |= STAnimate;
    }
    if (a1->GroupFlags & VisTray)
        a1->ObjFlags |= 0x400;
    if (a1->name.isEmpty())
        qDebug() << "No name in trick";
    auto iter = tricks_string_hash_tab.find(a1->name.toLower());
    if (iter!=tricks_string_hash_tab.end())
    {
        qDebug() << "duplicate model trick!";
        return;
    }
    tricks_string_hash_tab[a1->name.toLower()]=a1;
}
static void  trickLoadPostProcess(AllTricks_Data *a2)
{
    g_texture_path_to_mod.clear();
    tricks_string_hash_tab.clear();
    for (TextureModifiers &texopt : a2->texture_mods)
        setupTexOpt(&texopt);
    for (GeometryModifiers &trickinfo : a2->geometry_mods)
        setupTrick(&trickinfo);
}
bool loadTricksBin()
{
    BinStore binfile;
    QString fname(basepath+"bin/tricks.bin");
    {
        if (!(binfile.open(fname, tricks_i0_requiredCrc) || binfile.open(fname, tricks_i2_requiredCrc)))
        {
            qCritical() << "Failed to open original bin:" << fname;
            return false;
        }
        if (!loadFrom(&binfile, &s_tricks_store))
        {
            qCritical() << "Failed to open data from original bin:" << fname;
            return false;
        }
    }
    trickLoadPostProcess(&s_tricks_store);
    return true;
}

void convertTextureNames(const int *a1, std::vector<QString> &a2)
{
    int   num_textures          = a1[0];
    const int * indices         = a1 + 1;
    const char *start_of_strings_area = (const char *)a1 + num_textures * 4 + sizeof(int);
    for (int idx = 0; idx < num_textures; ++idx)
    {
        // fixup the offsets by adding the end of index area
        a2.push_back(start_of_strings_area + indices[idx]);
    }
}
std::vector<TextureBind> convertTexBinds(int cnt, const uint8_t *data)
{
    std::vector<TextureBind> res;
    res.assign((const TextureBind *)data,((const TextureBind *)data)+cnt);
    return res;
}
static ConvertedModel *convertAndInsertModel(ConvertedGeoSet &tgt, const Model32 *v)
{
    ConvertedModel *z = new ConvertedModel;

    z->flags             = v->flg1;
    z->visibility_radius = v->radius;
    z->num_textures      = v->num_textures;
    z->boneinfo_offset   = v->boneinfo;
    z->blend_mode        = CoHBlendMode(v->blend_mode);
    z->vertex_count      = v->vertex_count;
    z->model_tri_count   = v->model_tri_count;
    z->scale             = v->m_scale;
    z->m_min             = v->m_min;
    z->m_max             = v->m_max;
    for (uint8_t i = 0; i < 7; ++i)
    {
        DeltaPack &dp_blk(z->packed_data[i]);
        dp_blk.compressed_size = v->pack_data[i].compressed_size;
        dp_blk.uncomp_size     = v->pack_data[i].uncomp_size;
        dp_blk.compressed_data = nullptr;
        dp_blk.buffer_offset   = v->pack_data[i].compressed_data_off;
    }
    tgt.subs.push_back(z);
    return z;
}

void  addModelStubs(ConvertedGeoSet *geoset)
{
    for(ConvertedModel * m : geoset->subs)
    {
        GeometryModifiers *v3 = findGeomModifier(m->name, nullptr);
        if ( v3 )
        {
            if ( !m->trck_node )
                m->trck_node = new ModelModifiers;
            *m->trck_node = v3->node;
            m->trck_node->info = v3;
        }
    }
}
void geosetLoadHeader(QFile &fp, ConvertedGeoSet *geoset)
{
    unsigned int anm_hdr_size;
    const uint8_t * stream_pos_0;
    const uint8_t * stream_pos_1;
    uint32_t headersize;
    fp.read((char *)&anm_hdr_size, 4u);
    anm_hdr_size -= 4;
    fp.read((char *)&headersize, sizeof(uint32_t));

    QByteArray zipmem = fp.read(anm_hdr_size);
    QByteArray unc_arr = uncompr_zip(zipmem.data(), anm_hdr_size, headersize);

    const uint8_t * mem = (const uint8_t *)unc_arr.data();

    const TexBlockInfo *info = (const TexBlockInfo *)mem;
    geoset->geo_data_size = info->size1;

    convertTextureNames((const int *)(mem + sizeof(TexBlockInfo)), geoset->tex_names);
    stream_pos_0            = mem + info->texname_blocksize + sizeof(TexBlockInfo);
    stream_pos_1            = stream_pos_0 + info->bone_names_size;
    const GeosetHeader32 *header32  = (const GeosetHeader32 *)(stream_pos_1 + info->tex_binds_size);
    const Model32 *     ptr_subs  = (Model32 *)(stream_pos_1 + info->tex_binds_size + sizeof(GeosetHeader32));
    geoset->parent_geoset = geoset;
    geoset->name = header32->name;
    for (int idx = 0; idx < header32->num_subs; ++idx) {
        const Model32 *v6 = &ptr_subs[idx];
        std::vector<TextureBind> binds;
        if (info->tex_binds_size) {
            binds = convertTexBinds(v6->num_textures, v6->texture_bind_offsets + stream_pos_1);
        }
        ConvertedModel *m    = convertAndInsertModel(*geoset, v6);
        m->texture_bind_info = binds;
        m->geoset       = geoset;
        m->vbo          = nullptr;
        m->name         = QString((const char *)stream_pos_0 + v6->bone_name_offset);
    }
    if (!geoset->subs.empty())
        addModelStubs(geoset);
}
static ConvertedGeoSet *findAndPrepareGeoSet(const QString &fname)
{
    ConvertedGeoSet *geoset = nullptr;
    QFile fp;
    fp.setFileName(basepath+fname);
    if (fp.open(QFile::ReadOnly))
    {
        geoset = new ConvertedGeoSet;
        geoset->geopath = fname;
        geosetLoadHeader(fp, geoset);
        fp.seek(0);
        s_name_to_geoset[fname] = geoset;
    } else {
        qCritical() <<"Can't find .geo file"<<fname;
    }
    return geoset;
}
ConvertedModel *modelFind(const QString &model_name, const QString &filename)
{
    ConvertedModel *ptr_sub = nullptr;

    if (model_name.isEmpty() || filename.isEmpty()) {
        qCritical() << "Bad model/geometry set requested:";
        if (!model_name.isEmpty())
            qCritical() << "Model: "<<model_name;
        if (!filename.isEmpty())
            qCritical() << "GeoFile: "<<filename;
        return nullptr;
    }
    ConvertedGeoSet *geoset = geosetLoad(filename);
    if (!geoset) // failed to load the geometry set
        return nullptr;
    int end_of_name_idx  = model_name.indexOf("__");
    if (end_of_name_idx == -1)
        end_of_name_idx = model_name.size();
    QStringRef basename(model_name.midRef(0, end_of_name_idx));

    for (ConvertedModel *m : geoset->subs)
    {
        QString geo_name = m->name;
        if (geo_name.isEmpty())
            continue;
        bool subs_in_place = (geo_name.size() <= end_of_name_idx || geo_name.midRef(end_of_name_idx).startsWith("__"));
        if (subs_in_place && geo_name.startsWith(basename, Qt::CaseInsensitive))
            ptr_sub = m; // TODO: return immediately
    }
    return ptr_sub;

}
void initLoadedModel(Urho3D::Context *ctx,Urho3D::Model *m,ConvertedModel *model,const std::vector<TextureWrapper> &textures)
{
    model->blend_mode = CoHBlendMode::MULTIPLY_REG;
    bool isgeo=false;
    if(model->name.startsWith("GEO_",Qt::CaseInsensitive))
    {
        model->flags |= 0x4000;
        isgeo = true;
        if ( model->name.contains("eyes",Qt::CaseInsensitive) )
        {
            if ( !model->trck_node )
                model->trck_node = new ModelModifiers;
            model->trck_node->_TrickFlags |= DoubleSided;
        }
    }
    assert(model->num_textures==model->texture_bind_info.size());
    for(TextureBind tbind : model->texture_bind_info)
    {
        const TextureWrapper &texb(textures[tbind.tex_idx]);
        if (!isgeo)
        {
            TextureWrapper seltex     = texb;
            uint32_t       extra_flag = 0;
            if (texb.info)
            {
                auto blend = CoHBlendMode(texb.info->BlendType);
                if (blend == CoHBlendMode::ADDGLOW || blend == CoHBlendMode::COLORBLEND_DUAL ||
                    blend == CoHBlendMode::ALPHADETAIL)
                    seltex = tryLoadTexture(ctx, texb.detailname);
                if (seltex.base && seltex.flags & TextureWrapper::ALPHA)
                    extra_flag = 1;
            }
            model->flags |= extra_flag;
        }
        if ( texb.flags & TextureWrapper::DUAL )
        {
            model->flags |= 0x40;
            if ( texb.BlendType != CoHBlendMode::MULTIPLY )
                model->blend_mode = texb.BlendType;
        }
        if ( !texb.bumpmap.isEmpty() )
        {
            auto wrap(tryLoadTexture(ctx,texb.bumpmap));
            if ( wrap.flags & TextureWrapper::BUMPMAP )
            {
                model->flags |= 0x800;
                model->blend_mode = (model->blend_mode == CoHBlendMode::COLORBLEND_DUAL) ?
                            CoHBlendMode::BUMPMAP_COLORBLEND_DUAL : CoHBlendMode::BUMPMAP_MULTIPLY;
            }
            if ( texb.flags & TextureWrapper::CUBEMAPFACE || (wrap.flags & TextureWrapper::CUBEMAPFACE) )
                model->flags |= 0x2000;
        }
    }
    if ( model->trck_node && model->trck_node->info)
    {

        model->flags |= model->trck_node->info->ObjFlags;
    }
    if ( model->blend_mode == CoHBlendMode::COLORBLEND_DUAL || model->blend_mode == CoHBlendMode::BUMPMAP_COLORBLEND_DUAL )
    {
        if ( !model->trck_node )
            model->trck_node = new ModelModifiers;
        model->trck_node->_TrickFlags |= SetColor;
    }
    if ( model->blend_mode == CoHBlendMode::ADDGLOW )
    {
        if ( !model->trck_node )
            model->trck_node = new ModelModifiers;
        model->trck_node->_TrickFlags |= SetColor | NightLight;
    }
    if ( !model->packed_data.norms.uncomp_size ) // no normals
        model->flags |= 4; // only ambient light
    if ( model->trck_node  && model->trck_node->_TrickFlags & Additive )
        model->flags |= 1; // alpha pass
    if ( model->flags & 0x400 ) // force opaque
        model->flags &= ~1;
    if ( model->trck_node && model->trck_node->info)
    {
        if ( model->trck_node->info->blend_mode )
            model->blend_mode = CoHBlendMode(model->trck_node->info->blend_mode);
    }
    if(m)
        s_coh_model_to_renderable[model] = m;
}
ptrdiff_t unpackedDeltaPack(int *tgt_buf, uint8_t *data, uint32_t entry_size, uint32_t num_entries, UnpackMode v_type)
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
    if (inv_scale != 0.0f)
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
                if (extracted_2bits_ == 3)
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

void geoUnpackDeltas(DeltaPack *a1, uint8_t *target, uint32_t entry_size, uint32_t num_entries, UnpackMode type)
{
    if (0 == a1->uncomp_size)
        return;
    ptrdiff_t consumed_bytes;
    if (a1->compressed_size)
    {
        QByteArray unpacked = uncompr_zip((char *)a1->compressed_data, a1->compressed_size, a1->uncomp_size);
        consumed_bytes = unpackedDeltaPack((int *)target, (uint8_t *)unpacked.data(), entry_size, num_entries, type);
    }
    else
    {
        consumed_bytes = unpackedDeltaPack((int *)target, (uint8_t *)a1->compressed_data, entry_size, num_entries, type);
    }
}
inline void geoUnpackDeltas(DeltaPack *a1, Vector3 *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(a1, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_FLOATS);
}
inline void geoUnpackDeltas(DeltaPack *a1, Vector3i *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(a1, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_INTS);
}

void *fillVbo(ConvertedModel *model)
{
    VBOPointers &vbo(*model->vbo);
    std::vector<Vector3i> &triangles(model->vbo->triangles);
    triangles.resize(model->model_tri_count);//, 1, ".\\render\\model_cache.c", 138);
    geoUnpackDeltas(&model->packed_data.tris, triangles.data(), model->model_tri_count);
    uint32_t total_size = 0;
    uint32_t Vertices3D_bytes = sizeof(Vector3) * model->vertex_count;
    total_size += Vertices3D_bytes;
    if (model->packed_data.norms.uncomp_size)
        total_size += Vertices3D_bytes;
    if ( model->packed_data.sts.uncomp_size )
        total_size += 2*sizeof(Vector2) * model->vertex_count;

    float *v12 = (float *)calloc(total_size, 1);
    float *databuf=v12;
    vbo.pos = (Vector3 *)v12;

    v12 += Vertices3D_bytes/sizeof(float);
    geoUnpackDeltas(&model->packed_data.verts, vbo.pos, model->vertex_count);
    if (model->packed_data.norms.uncomp_size)
    {
        vbo.norm = (Vector3 *)v12;
        v12 += Vertices3D_bytes/sizeof(float);
        geoUnpackDeltas(&model->packed_data.norms, vbo.norm, model->vertex_count);
    }
    if (model->packed_data.sts.uncomp_size)
    {
        vbo.uv1 = (Vector2 *)v12;
        v12 += model->vertex_count * sizeof(Vector2)/sizeof(float);
        geoUnpackDeltas(&model->packed_data.sts, (uint8_t *)vbo.uv1, 2, model->vertex_count, UNPACK_FLOATS);
        vbo.uv2 = (Vector2 *)v12;
        v12 += model->vertex_count * sizeof(Vector2)/sizeof(float);
        memcpy(vbo.uv2, vbo.uv1, sizeof(Vector2) * model->vertex_count);
    }
    return databuf;
}
void modelFixup(ConvertedModel *model,VBOPointers &vbo)
{
    if (vbo.norm && (model->flags & 0x10))
    {
        for (uint32_t i = 0; i<model->vertex_count; ++i)
            vbo.norm[i] = Vector3(1, -1, 1);
    }

    if ( !vbo.uv1)
        return;

    bool texture_scaling_used = false;
    for (uint32_t i = 0; i < model->vertex_count; ++i )
    {
        vbo.uv1[i].y_ = 1.0f - vbo.uv1[i].y_;
    }
    if ( vbo.uv2 )
    {
        for (uint32_t i = 0; i < model->vertex_count; ++i)
        {
            vbo.uv2[i].y_ = 1.0f - vbo.uv2[i].y_;
        }
    }
    for(const TextureWrapper &tex : vbo.assigned_textures)
    {
        if(!tex.info)
            continue;

        if ( 1.0f != tex.scaleUV0.x_ || 1.0f != tex.scaleUV0.y_ ||
             1.0f != tex.scaleUV1.x_ || 1.0f != tex.scaleUV1.y_ )
            texture_scaling_used = true;
    }
    if ( !texture_scaling_used )
        return;

    std::vector<bool> vertex_uv_was_scaled(model->vertex_count);
    uint32_t triangle_offset = 0;
    for (uint32_t j = 0; j < model->num_textures; ++j )
    {
        const TextureWrapper &tex(vbo.assigned_textures[j]);
        const uint32_t bind_tri_count = model->texture_bind_info[j].tri_count;
        if(!tex.info)
            continue;
        Vector2 scaletex0 = tex.scaleUV0;
        Vector2 scaletex1 = tex.scaleUV1;
        for (uint32_t v19 = 0; v19 < bind_tri_count; ++v19)
        {
            Vector3i tri(vbo.triangles[v19+triangle_offset]);
            for(int vnum=0; vnum<3; ++vnum)
            {
                const uint32_t vert_idx = tri[vnum];
                if (vertex_uv_was_scaled[vert_idx])
                    continue;
                vertex_uv_was_scaled[vert_idx] = true;
                vbo.uv2[vert_idx].x_ *= scaletex0.x_;
                vbo.uv2[vert_idx].y_ *= scaletex0.y_;
                vbo.uv1[vert_idx].x_ *= scaletex1.x_;
                vbo.uv1[vert_idx].y_ *= scaletex1.y_;
            }
        }
        triangle_offset += bind_tri_count;
    }
}
void reportUnhandled(const QString &message)
{
    static QSet<QString> already_reported;
    if(already_reported.contains(message))
        return;
    qDebug() << message;
    already_reported.insert(message);
}
float *combineBuffers(VBOPointers *meshdata,ConvertedModel *mdl)
{
    size_t num_floats = mdl->vertex_count*3;
    if(meshdata->norm)
        num_floats += mdl->vertex_count*3;
    if(meshdata->uv1)
        num_floats += mdl->vertex_count*2;
    if(meshdata->uv2)
        num_floats += mdl->vertex_count*2;

    float *res = new float[num_floats];
    int off=0;
    for(uint32_t i=0; i<mdl->vertex_count; ++i) {
        res[off++] = meshdata->pos[i].x_;
        res[off++] = meshdata->pos[i].y_;
        res[off++] = meshdata->pos[i].z_;
        if(meshdata->norm) {
            res[off++] = meshdata->norm[i].x_;
            res[off++] = meshdata->norm[i].y_;
            res[off++] = meshdata->norm[i].z_;
        }
        if(meshdata->uv1)
        {
            res[off++] = meshdata->uv1[i].x_;
            res[off++] = meshdata->uv1[i].y_;
        }
        if(meshdata->uv2)
        {
            res[off++] = meshdata->uv2[i].x_;
            res[off++] = meshdata->uv2[i].y_;
        }
    }
    return res;
}
void modelCreateObjectFromModel(Urho3D::Context *ctx,ConvertedModel *model,std::vector<TextureWrapper> &textures)
{
    initLoadedModel(ctx,nullptr,model,textures);
    model->vbo = new VBOPointers;
    VBOPointers &vbo(*model->vbo);
    vbo.assigned_textures.reserve(textures.size());
    for(TextureBind tbind : model->texture_bind_info)
    {
        TextureWrapper &texb(textures[tbind.tex_idx]);
        vbo.assigned_textures.emplace_back(texb);
    }
    void *databuf = fillVbo(model);
    modelFixup(model,vbo);


    SharedPtr<Urho3D::Model> fromScratchModel(new Urho3D::Model(ctx));
    SharedPtr<VertexBuffer> vb(new VertexBuffer(ctx));
    SharedPtr<IndexBuffer> ib(new IndexBuffer(ctx));
    std::vector<VertexElement> vertex_elements;
    vertex_elements.emplace_back(TYPE_VECTOR3, SEM_POSITION);
    if(model->packed_data.norms.uncomp_size)
        vertex_elements.emplace_back(TYPE_VECTOR3, SEM_NORMAL);
    if(vbo.uv1)
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD);
    if(vbo.uv2)
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD);
    float *combined = combineBuffers(&vbo,model);
    free(databuf);
    vb->SetShadowed(true);
    vb->SetSize(model->vertex_count, vertex_elements);
    vb->SetData(combined);
    delete [] combined;
    ib->SetShadowed(true);
    ib->SetSize(model->model_tri_count*3, true);
    ib->SetData(vbo.triangles.data());
    fromScratchModel->SetVertexBuffers({vb},{},{});
    fromScratchModel->SetIndexBuffers({ib});
    BoundingBox bbox(model->m_min,model->m_max);
    unsigned geom_count = model->texture_bind_info.size();
    fromScratchModel->SetNumGeometries(geom_count);
    unsigned face_offset=0;
    for(unsigned i=0; i<geom_count; ++i)
    {
        const TextureBind &tbind(model->texture_bind_info[i]);
        fromScratchModel->SetNumGeometryLodLevels(i, 1);
        SharedPtr<Geometry> geom(new Geometry(ctx));
        geom->SetVertexBuffer(0, vb);
        geom->SetIndexBuffer(ib);
        geom->SetDrawRange(TRIANGLE_LIST, face_offset, tbind.tri_count*3);
        fromScratchModel->SetGeometry(i, 0, geom);
        face_offset+=tbind.tri_count*3;
    }
    assert(face_offset==model->model_tri_count*3);
    fromScratchModel->SetBoundingBox(bbox);
    s_coh_model_to_renderable[model] = fromScratchModel;
}
void addModelData(Urho3D::Context *ctx,ConvertedGeoSet *geoset)
{
    std::vector<TextureWrapper> v2 = getModelTextures(ctx,geoset->tex_names);
    for(ConvertedModel * model : geoset->subs)
    {
        modelCreateObjectFromModel(ctx,model, v2);
    }
}
void fixupDataPtr(DeltaPack &a, uint8_t *b)
{
    if (a.uncomp_size)
        a.compressed_data = b + a.buffer_offset;
}

void geosetLoadData(Urho3D::Context *ctx,QFile &fp, ConvertedGeoSet *geoset)
{
    int buffer;
    fp.seek(0);
    fp.read((char *)&buffer, 4);
    fp.seek(buffer + 8);
    geoset->m_geo_data.resize(geoset->geo_data_size); //, 1, "\\src\\Common\\seq\\anim.c", 496);
    fp.read(geoset->m_geo_data.data(), geoset->geo_data_size);
    uint8_t *buffer_b = (uint8_t *)geoset->m_geo_data.data();

    for (ConvertedModel *current_sub : geoset->subs)
    {
        fixupDataPtr(current_sub->packed_data.tris, buffer_b);
        fixupDataPtr(current_sub->packed_data.verts, buffer_b);
        fixupDataPtr(current_sub->packed_data.norms, buffer_b);
        fixupDataPtr(current_sub->packed_data.sts, buffer_b);
        fixupDataPtr(current_sub->packed_data.grid, buffer_b);
        fixupDataPtr(current_sub->packed_data.weights, buffer_b);
        fixupDataPtr(current_sub->packed_data.matidxs, buffer_b);
        if (current_sub->boneinfo_offset)
        {
            qCritical() << "Models with bones are not supported yet, bother SEGS devs to fix that";
            assert(false);
        }
    }
    if (!geoset->subs.empty())
        addModelData(ctx,geoset);
    geoset->data_loaded = true;
}

Urho3D::Model *buildModel(Urho3D::Context *ctx,ConvertedModel *mdl)
{
    ResourceCache* cache = ctx->m_ResourceCache.get();
    auto parts=mdl->geoset->geopath.split('/');
    while(!parts.isEmpty())
    {
        QString str=parts.takeFirst();
        if(str.compare(QLatin1Literal("object_library"),Qt::CaseInsensitive)==0)
            break;
    }
    QString cache_path="converted/Models/"+parts.join('/')+"/"+mdl->name+".mdl";
    if(cache->Exists(cache_path))
    {
        auto lf3d_model=cache->GetResource<Model>(cache_path);
        if(lf3d_model)
        {
            std::vector<TextureWrapper> v2 = getModelTextures(ctx,mdl->geoset->tex_names);
            initLoadedModel(ctx,lf3d_model,mdl,v2);
            return lf3d_model;
        }
    }
    QFile fl(basepath+mdl->geoset->geopath);
    if(!fl.exists() || !fl.open(QFile::ReadOnly) || s_coh_model_to_renderable.size()>5000)
    {
        if(s_coh_model_to_renderable.size()<=3) // this will report first few missing geometry files
        {
            qDebug() << "Missing geo file" << basepath+mdl->geoset->geopath;
        }
        return nullptr;
    }
    if(!mdl->geoset->data_loaded) {
        geosetLoadData(ctx,fl,mdl->geoset);
    }
    QDir modeldir("converted/Models");
    modeldir.mkpath(parts.join('/'));
    auto res=s_coh_model_to_renderable[mdl];
    File model_res(ctx,cache_path,FILE_WRITE);
    res->Save(model_res);
    return res;
}

void convertMaterial(Urho3D::Context *ctx,ConvertedModel *mdl,StaticModel* boxObject)
{
    ModelModifiers *  model_trick = mdl ? mdl->trck_node : nullptr;
    ResourceCache *   cache       = ctx->m_ResourceCache.get();
    SharedPtr<Material> result;
    QString model_base_name = mdl->name.split("__").front();
    //result = cache->GetResource<Material>("Materials/WireFrame.xml");
    bool isDoubleSided = model_trick && model_trick->isFlag(DoubleSided);
    if(model_trick&&model_trick->isFlag(ColorOnly))
    {
//        result = result->Clone(result->GetName()+"Colored");
//        result->SetShaderParameter("MatDiffColor",Vector4(1.0, 1.0, 0.2f, 1.0f));
    }
    // Select material based on the model blend state
    // Modify material based on the applied model tricks
    Color onlyColor;
    Vector4 tint1 = {1,1,1,1}; // Shader Constant 0
    Vector4 tint2 = {1,1,1,1}; // Shader Constant 1
    CullMode targetCulling=CULL_CCW;
    CompareMode depthTest = CMP_LESSEQUAL;
    float alphaRef = 0.0f;
    bool depthWrite = true;
    bool isAdditive=false;
    if(model_trick && model_trick->_TrickFlags)
    {
        auto tflags = model_trick->_TrickFlags;
        if ( tflags & Additive )
        {
            isAdditive = true;
        }
        if ( tflags & ColorOnly )
            onlyColor = Color(model_trick->TintColor0.r/255.0f, model_trick->TintColor0.g/255.0f, model_trick->TintColor0.b/255.0f, model_trick->TintColor0.a/255.0f);
        if ( tflags & DoubleSided )
            targetCulling = CULL_NONE;
        if ( tflags & NoZTest )
        {
            // simulate disabled Z test
            depthTest = CMP_ALWAYS;
            depthWrite = false;
        }
        if(tflags & NoZWrite)
            depthWrite = false;
        if ( tflags & SetColor )
        {
            glm::vec3 tnt0=model_trick->TintColor0.to3Floats();
            glm::vec3 tnt1=model_trick->TintColor1.to3Floats();
            tint1 = Vector4(tnt0.r,tnt0.g,tnt0.b,1.0f);
            tint1 = Vector4(tnt1.r,tnt1.g,tnt1.b,1.0f);
        }
        if ( tflags & (ReflectTex0 | ReflectTex1) ) {
            reportUnhandled("Unhandled cubemap reflection");
        }
        if ( tflags & AlphaRef ) {
            //qDebug() << "Unhandled alpha ref";
            alphaRef = model_trick->info->AlphaRef;
        }
        if ( tflags & TexBias )
            reportUnhandled("Unhandled TexBias");
    }
    auto whitetex = tryLoadTexture(ctx,"white.tga");
    QStringList vertex_defines;
    QStringList pixel_defines;
    SharedPtr<Material> preconverted;
    if(cache->Exists("./converted/Materials/"+model_base_name+"_mtl.xml"))
        preconverted = cache->GetResource<Material>("./converted/Materials/"+model_base_name+"_mtl.xml");
    bool noLightAngle= mdl->flags & 0x10;
    if(!preconverted) {
        if(mdl->flags&0x100)
        {
            preconverted = cache->GetResource<Material>("Materials/DefaultVegetation.xml")->Clone();
        }
        else {
            if(noLightAngle)
                preconverted = cache->GetResource<Material>("Materials/NoLightAngle.xml")->Clone();
            else
                preconverted = cache->GetResource<Material>("Materials/TexturedDual.xml")->Clone();

        }
    }
    else {
        preconverted=preconverted->Clone();
    }
    pixel_defines << "DIFFMAP" << "ALPHAMASK";
    switch(mdl->blend_mode)
    {
    case CoHBlendMode::MULTIPLY:
        pixel_defines << "COH_MULTIPLY";
        break;
    case CoHBlendMode::MULTIPLY_REG:
        if(!depthWrite && isAdditive) {
            preconverted = cache->GetResource<Material>("Materials/AddAlpha.xml")->Clone();
            preconverted->SetCullMode(CULL_NONE);
        }
        pixel_defines << "COH_MULTIPLY_REG";
        break;
    case CoHBlendMode::COLORBLEND_DUAL:
        pixel_defines << "COH_COLOR_BLEND_DUAL";
        break;
    case CoHBlendMode::ADDGLOW:
        pixel_defines << "COH_ADDGLOW";
        break;
    case CoHBlendMode::ALPHADETAIL:
        qDebug() << "unhandled ALPHADETAIL";
        break;
    case CoHBlendMode::BUMPMAP_MULTIPLY:
        preconverted = cache->GetResource<Material>("Materials/TexturedDualBump.xml")->Clone();
        pixel_defines << "COH_MULTIPLY";
        break;
    case CoHBlendMode::BUMPMAP_COLORBLEND_DUAL:
        preconverted = cache->GetResource<Material>("Materials/TexturedDualBump.xml")->Clone();
        pixel_defines << "COH_COLOR_BLEND_DUAL";
        break;
    }
    if(model_trick && model_trick->isFlag(SetColor)) {
        reportUnhandled("SetColor unhandled");
    }
    if(mdl->flags&0x100) {
        preconverted->SetVertexShaderDefines("TRANSLUCENT");
        pixel_defines<<"TRANSLUCENT";
        preconverted->SetShaderParameter("AlphaRef",0.4f);
        tint1.w_ = 254/255.0f;
    }
    else if(alphaRef!=0.0f)
        preconverted->SetShaderParameter("AlphaRef",alphaRef);
    preconverted->SetShaderParameter("Col1",tint1);
    preconverted->SetShaderParameter("Col2",tint2);
    preconverted->SetPixelShaderDefines(pixel_defines.join(' '));
    if(isDoubleSided)
        preconverted->SetCullMode(CULL_NONE);

    // int mode= dualTexture ? 4 : 5
    unsigned geomidx=0;
    for(auto & texbind : mdl->texture_bind_info) {
        const QString &texname(mdl->geoset->tex_names[texbind.tex_idx]);
        TextureWrapper tex = tryLoadTexture(ctx,texname);
        if(tex.base) {
            result = preconverted->Clone();
            result->SetTexture(TU_DIFFUSE,tex.base);
            if(tex.info) {
                if(!tex.info->Blend.isEmpty())
                {
                    TextureWrapper detail = tryLoadTexture(ctx,tex.info->Blend);
                    result->SetTexture(TU_CUSTOM1,detail.base);
                }
                else
                    result->SetTexture(TU_CUSTOM1,whitetex.base);

                if(!tex.info->BumpMap.isEmpty())
                {
                    TextureWrapper normal = tryLoadTexture(ctx,tex.info->BumpMap);
                    result->SetTexture(TU_NORMAL,normal.base);
                }
            }
            else
                result->SetTexture(TU_CUSTOM1,whitetex.base);

            boxObject->SetMaterial(geomidx,result);
        }
        geomidx++;
    }

}

} // end of anonymus namespace
Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, ConvertedNode *node, int opt)
{
    ConvertedModel *mdl = node->model;
    ModelModifiers *model_trick = mdl->trck_node;
    if(model_trick) {
        if(model_trick->isFlag(NoDraw)) {
            //qDebug() << mdl->name << "Set as no draw";
            return nullptr;
        }
        if(opt!=CONVERT_EDITOR_MARKERS && model_trick->isFlag(EditorVisible)) {
            //qDebug() << mdl->name << "Set as editor model";
            return nullptr;
        }
        if(model_trick && model_trick->isFlag(CastShadow)) {
            //qDebug() << "Not converting shadow models"<<mdl->name;
            return nullptr;
        }
        if(model_trick && model_trick->isFlag(ParticleSys)) {
            qDebug() << "Not converting particle sys:"<<mdl->name;
            return nullptr;
        }
    }
    auto modelptr = buildModel(ctx,mdl);
    if(!modelptr)
        return nullptr;
    StaticModel* boxObject = tgtnode->CreateComponent<StaticModel>();
    boxObject->SetDrawDistance(node->lod_far+node->lod_far_fade);
    modelptr->SetName(mdl->name);
    boxObject->SetModel(modelptr);
    convertMaterial(ctx,mdl,boxObject);
    return boxObject;
}

ConvertedModel *groupModelFind(const QString & a1)
{
    QString model_name=a1.mid(a1.lastIndexOf('/')+1);
    auto val = groupGetFileEntryPtr(model_name);
    return val ? modelFind(model_name,val->geopath) : nullptr;
}

bool prepareGeoLookupArray()
{
    QFile defnames(basepath+"bin/defnames.bin");
    if(!defnames.open(QFile::ReadOnly))
    {
        qCritical() << "Failed to open defnames.bin";
        return false;
    }
    QByteArrayList defnames_arr;
    for(const QByteArray &entr : defnames.readAll().split('\0'))
        defnames_arr.push_back(entr);
    QString lookup_str;
    GeoStoreDef *current_geosetinf=nullptr;
    for (QString str : defnames_arr )
    {
        str.replace("CHUNKS.geo","Chunks.geo"); // original paths are case insensitive
        int last_slash = str.lastIndexOf('/');
        if(-1!=last_slash)
        {
            QString geo_path = str.mid(0,last_slash);
            lookup_str = geo_path.toLower();
            current_geosetinf = &s_dir_to_geoset[lookup_str];
            current_geosetinf->geopath = geo_path;
        }
        current_geosetinf->entries << str.mid(last_slash + 1);
        s_modelname_to_geostore[str.mid(last_slash + 1)] = current_geosetinf;
    }

    return loadTricksBin();
}
GeometryModifiers *findGeomModifier(const QString &modelname, const QString &trick_path)
{
    QStringList parts = modelname.split("__");
    if ( parts.size()<2 )
        return nullptr;
    parts.removeFirst();
    QString bone_trick_name = parts.join("__");
    GeometryModifiers *result = tricks_string_hash_tab.value(bone_trick_name.toLower(),nullptr);
    if ( result )
        return result;
    qDebug() << "Can't find modifier for" << trick_path<<modelname;
    return nullptr;
}
GeoStoreDef * groupGetFileEntryPtr(const QString &a1)
{
    QString key = a1.mid(a1.lastIndexOf('/')+1);
    key = key.mid(0,key.indexOf("__"));
    return s_modelname_to_geostore.value(key,nullptr);
}
/// load the given geoset, used when loading scene-subgraph and nodes
ConvertedGeoSet * geosetLoad(const QString &m)
{
    ConvertedGeoSet * res = s_name_to_geoset.value(m,nullptr);
    if(res)
        return res;
    return findAndPrepareGeoSet(m);
}
