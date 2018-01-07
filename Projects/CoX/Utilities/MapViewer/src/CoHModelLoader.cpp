#include "CoHModelLoader.h"

#include "CohModelConverter.h"
#include "CoHSceneConverter.h"
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
glm::vec3 fromUrho(Vector3 v) { return {v.x_,v.y_,v.z_};}
Vector3 toUrho(glm::vec3 v) { return {v.x,v.y,v.z};}

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

AllTricks_Data s_tricks_store;
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
static CoHModel *convertAndInsertModel(ConvertedGeoSet &tgt, const Model32 *v)
{
    CoHModel *z = new CoHModel;

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
    for(CoHModel * m : geoset->subs)
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
        CoHModel *m    = convertAndInsertModel(*geoset, v6);
        m->texture_bind_info = binds;
        m->geoset       = geoset;
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
CoHModel *modelFind(const QString &model_name, const QString &filename)
{
    CoHModel *ptr_sub = nullptr;

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

    for (CoHModel *m : geoset->subs)
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

void geoUnpackDeltas(const DeltaPack *a1, uint8_t *target, uint32_t entry_size, uint32_t num_entries, UnpackMode type)
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
inline void geoUnpackDeltas(const DeltaPack *a1, glm::vec3 *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(a1, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_FLOATS);
}
inline void geoUnpackDeltas(const DeltaPack *a1, glm::ivec3 *unpacked_data, uint32_t num_entries)
{
    geoUnpackDeltas(a1, (uint8_t *)unpacked_data, 3, num_entries, UNPACK_INTS);
}
static bool bumpMapped(const CoHModel &model) {
    return model.flags & (OBJ_DRAW_AS_ENT | OBJ_BUMPMAP);
}

std::unique_ptr<VBOPointers> fillVbo(const CoHModel &model)
{
    std::unique_ptr<VBOPointers> vbo(new VBOPointers);
    std::vector<glm::ivec3> &triangles(vbo->triangles);
    triangles.resize(model.model_tri_count);//, 1, ".\\render\\model_cache.c", 138);
    geoUnpackDeltas(&model.packed_data.tris, triangles.data(), model.model_tri_count);
    uint32_t total_size = 0;
    uint32_t Vertices3D_bytes = sizeof(Vector3) * model.vertex_count;
    total_size += Vertices3D_bytes;
    if (model.packed_data.norms.uncomp_size)
        total_size += Vertices3D_bytes;
    if ( model.packed_data.sts.uncomp_size )
        total_size += 2*sizeof(Vector2) * model.vertex_count;

    vbo->pos.resize(model.vertex_count);

    geoUnpackDeltas(&model.packed_data.verts, vbo->pos.data(), model.vertex_count);
    if (model.packed_data.norms.uncomp_size)
    {
        vbo->norm.resize(model.vertex_count);
        geoUnpackDeltas(&model.packed_data.norms, vbo->norm.data(), model.vertex_count);
    }
    if (model.packed_data.sts.uncomp_size)
    {
        vbo->uv1.resize(model.vertex_count);
        geoUnpackDeltas(&model.packed_data.sts, (uint8_t *)vbo->uv1.data(), 2, model.vertex_count, UNPACK_FLOATS);
        vbo->uv2 = vbo->uv1;
    }
    if(bumpMapped(model))
    {
        vbo->needs_tangents = true;
    }
    return vbo;
}
void modelFixup(const CoHModel &model,VBOPointers &vbo)
{
    if (!vbo.norm.empty() && (model.flags & OBJ_NOLIGHTANGLE))
    {
        for (uint32_t i = 0; i<model.vertex_count; ++i)
            vbo.norm[i] = glm::vec3(1, -1, 1);
    }

    if ( vbo.uv1.empty())
        return;

    bool texture_scaling_used = false;
    for (uint32_t i = 0; i < model.vertex_count; ++i )
    {
        vbo.uv1[i].y = 1.0f - vbo.uv1[i].y;
    }
    if ( !vbo.uv2.empty() )
    {
        for (uint32_t i = 0; i < model.vertex_count; ++i)
        {
            vbo.uv2[i].y = 1.0f - vbo.uv2[i].y;
        }
    }
    for(const TextureWrapper &tex : vbo.assigned_textures)
    {
        if(!tex.info)
            continue;

        if ( 1.0f != tex.scaleUV0.x || 1.0f != tex.scaleUV0.y ||
             1.0f != tex.scaleUV1.x || 1.0f != tex.scaleUV1.y )
            texture_scaling_used = true;
    }
    if ( !texture_scaling_used )
        return;

    std::vector<bool> vertex_uv_was_scaled(model.vertex_count);
    uint32_t triangle_offset = 0;
    for (uint32_t j = 0; j < model.num_textures; ++j )
    {
        const TextureWrapper &tex(vbo.assigned_textures[j]);
        const uint32_t bind_tri_count = model.texture_bind_info[j].tri_count;
        if(!tex.info)
            continue;
        glm::vec2 scaletex0 = tex.scaleUV0;
        glm::vec2 scaletex1 = tex.scaleUV1;
        for (uint32_t v19 = 0; v19 < bind_tri_count; ++v19)
        {
            glm::ivec3 tri(vbo.triangles[v19+triangle_offset]);
            for(int vnum=0; vnum<3; ++vnum)
            {
                const uint32_t vert_idx = tri[vnum];
                if (vertex_uv_was_scaled[vert_idx])
                    continue;
                vertex_uv_was_scaled[vert_idx] = true;
                vbo.uv2[vert_idx].x *= scaletex0.x;
                vbo.uv2[vert_idx].y *= scaletex0.y;
                vbo.uv1[vert_idx].x *= scaletex1.x;
                vbo.uv1[vert_idx].y *= scaletex1.y;
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
void fixupDataPtr(DeltaPack &a, uint8_t *b)
{
    if (a.uncomp_size)
        a.compressed_data = b + a.buffer_offset;
}

} // end of anonymus namespace

void geosetLoadData(QFile &fp, ConvertedGeoSet *geoset)
{
    int buffer;
    fp.seek(0);
    fp.read((char *)&buffer, 4);
    fp.seek(buffer + 8);
    geoset->m_geo_data.resize(geoset->geo_data_size); //, 1, "\\src\\Common\\seq\\anim.c", 496);
    fp.read(geoset->m_geo_data.data(), geoset->geo_data_size);
    uint8_t *buffer_b = (uint8_t *)geoset->m_geo_data.data();

    for (CoHModel *current_sub : geoset->subs)
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
    geoset->data_loaded = true;
}
CoHModel *groupModelFind(const QString & a1)
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

std::unique_ptr<VBOPointers> getVBO(CoHModel & model)
{
    std::unique_ptr<VBOPointers> databuf(fillVbo(model));
    modelFixup(model,*databuf);
    return databuf;
}
float *combineBuffers(VBOPointers &meshdata,CoHModel *mdl)
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

    float *res = new float[num_floats];
    int off=0;
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
    }
    if (meshdata.needs_tangents)
        GenerateTangents(res, offset, meshdata.triangles.data(), sizeof(uint32_t), 0,
                         3 * meshdata.triangles.size(), normal_offset, texcoord_offset, tangents_offset);
    return res;
}
void initLoadedModel(std::function<TextureWrapper(const QString &)> funcloader,CoHModel *model,const std::vector<TextureWrapper> &textures)
{
    model->blend_mode = CoHBlendMode::MULTIPLY_REG;
    bool isgeo=false;
    if(model->name.startsWith("GEO_",Qt::CaseInsensitive))
    {
        model->flags |= OBJ_DRAW_AS_ENT;
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
                    seltex = funcloader(texb.detailname);
                if (seltex.base && seltex.flags & TextureWrapper::ALPHA)
                    extra_flag = 1;
            }
            model->flags |= extra_flag;
        }
        if ( texb.flags & TextureWrapper::DUAL )
        {
            model->flags |= OBJ_DUALTEXTURE;
            if ( texb.BlendType != CoHBlendMode::MULTIPLY )
                model->blend_mode = texb.BlendType;
        }
        if ( !texb.bumpmap.isEmpty() )
        {
            auto wrap(funcloader(texb.bumpmap));
            if ( wrap.flags & TextureWrapper::BUMPMAP )
            {
                model->flags |= OBJ_BUMPMAP;
                model->blend_mode = (model->blend_mode == CoHBlendMode::COLORBLEND_DUAL) ?
                            CoHBlendMode::BUMPMAP_COLORBLEND_DUAL : CoHBlendMode::BUMPMAP_MULTIPLY;
            }
            if ( texb.flags & TextureWrapper::CUBEMAPFACE || (wrap.flags & TextureWrapper::CUBEMAPFACE) )
                model->flags |= OBJ_CUBEMAP;
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
        model->flags |= OBJ_FULLBRIGHT; // only ambient light
    if ( model->trck_node  && model->trck_node->_TrickFlags & Additive )
        model->flags |= OBJ_ALPHASORT; // alpha pass
    if ( model->flags & OBJ_FORCEOPAQUE ) // force opaque
        model->flags &= ~OBJ_ALPHASORT;
    if ( model->trck_node && model->trck_node->info)
    {
        if ( model->trck_node->info->blend_mode )
            model->blend_mode = CoHBlendMode(model->trck_node->info->blend_mode);
    }
}
