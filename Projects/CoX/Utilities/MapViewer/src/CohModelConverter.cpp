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

namespace {
AllTricks_Data g_tricks_store;
// name of the geometry is constructed from actual name and an optional modifer name
struct GeoStoreDef
{
    QString geopath;
    QStringList entries;
    bool loaded;
};
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
static QHash<QString,GeoStoreDef> s_dir_to_geoset;
static QHash<QString,GeoStoreDef *> s_modelname_to_geostore;
QHash<QString,GeometryModifiers *> tricks_string_hash_tab;
static QHash<QString,ConvertedGeoSet *> s_name_to_geoset;

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
static void  trickLoadPostProcess(int a1, AllTricks_Data *a2)
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
        if (!loadFrom(&binfile, &g_tricks_store))
        {
            qCritical() << "Failed to open data from original bin:" << fname;
            return false;
        }
    }
    trickLoadPostProcess(0,&g_tricks_store);
    return true;
}

GeoStoreDef * groupGetFileEntryPtr(const QString &a1)
{
    QString key = a1.mid(a1.lastIndexOf('/')+1);
    key = key.mid(0,key.indexOf("__"));
    return s_modelname_to_geostore.value(key,nullptr);
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
std::vector<TextureBind> convertTexBinds(int cnt, uint8_t *data)
{
    std::vector<TextureBind> res;
    res.assign((TextureBind *)data,((TextureBind *)data)+cnt);
    return res;
}
static ConvertedModel *convertAndInsertModel(ConvertedGeoSet &tgt, Model32 *v)
{
    ConvertedModel *z = new ConvertedModel;

    z->flags             = v->flg1;
    z->visibility_radius = v->radius;
    z->num_textures      = v->num_textures;
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
    const char * stream_pos_0;
    const char * stream_pos_1;
    uint32_t headersize;
    fp.read((char *)&anm_hdr_size, 4u);
    anm_hdr_size -= 4;
    fp.read((char *)&headersize, sizeof(uint32_t));

    QByteArray zipmem = fp.read(anm_hdr_size);
    QByteArray unc_arr = uncompr_zip(zipmem.data(), anm_hdr_size, headersize);

    const char * mem = unc_arr.data();

    const TexBlockInfo *info = (const TexBlockInfo *)mem;
    geoset->geo_data_size = info->size1;

    convertTextureNames((const int *)(mem + sizeof(TexBlockInfo)), geoset->tex_names);
    stream_pos_0            = mem + info->texname_blocksize + sizeof(TexBlockInfo);
    stream_pos_1            = stream_pos_0 + info->bone_names_size;
    GeosetHeader32 *header32  = (GeosetHeader32 *)(stream_pos_1 + info->tex_binds_size);
    Model32 *     ptr_subs  = (Model32 *)(stream_pos_1 + info->tex_binds_size + sizeof(GeosetHeader32));
    geoset->parent_geoset = geoset;
    geoset->name = header32->name;
    for (int idx = 0; idx < header32->num_subs; ++idx) {
        Model32 *v6 = &ptr_subs[idx];
        v6->vbo     = 0;
        std::vector<TextureBind> binds;
        if (info->tex_binds_size) {
            binds = convertTexBinds(v6->num_textures, v6->texture_bind_offsets + (uint8_t *)stream_pos_1);
        }
        ConvertedModel *m    = convertAndInsertModel(*geoset, v6);
        m->texture_bind_info = binds;
        m->geoset       = geoset;
        m->vbo          = nullptr;
        m->name         = QString(stream_pos_0 + v6->bone_name_offset);
    }
    if (!geoset->subs.empty())
        addModelStubs(geoset);
}
static ConvertedGeoSet *findAndPrepareGeoSet(const QString &fname)
{
    ConvertedGeoSet *geoset = nullptr;
    QFile fp;
    fp.setFileName(basepath+fname);
    if (fp.open(QFile::ReadOnly)) {
        geoset = new ConvertedGeoSet;
        geoset->geopath = fname;
        geosetLoadHeader(fp, geoset);
        fp.seek(0);
        s_name_to_geoset[fname] = geoset;
    } else {
        qCritical() <<"Cant find .geo file"<<fname;
    }
    return geoset;
}
ConvertedGeoSet * geosetLoad(const QString &m)
{
    ConvertedGeoSet * res = s_name_to_geoset.value(m,nullptr);
    if(res)
        return res;
    return findAndPrepareGeoSet(m);
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
    QStringRef name_without_mods=&model_name;
    int name_end_idx  = model_name.indexOf("__");
    if (name_end_idx != -1)
        name_without_mods = name_without_mods.mid(0,name_end_idx);

    for (ConvertedModel *m : geoset->subs)
    {
        QString geo_name = m->name;
        bool subs_in_place = (geo_name.size() <= name_end_idx || geo_name.midRef(name_end_idx).startsWith("__"));
        if (!geo_name.isEmpty()) {
            if (subs_in_place && geo_name.startsWith(name_without_mods, Qt::CaseInsensitive))
                ptr_sub = m; // TODO: return immediately ?
        }
    }
    return ptr_sub;

}
} // end of anonymus namespace
Urho3D::StaticModel *convertToLutefiskModel(Urho3D::Context *ctx, Urho3D::Node *tgtnode, ConvertedNode *def) 
{
    return nullptr;
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
    QString geo_path;
    GeoStoreDef *current_geosetinf;
    for (QString str : defnames_arr )
    {
        str.replace("CHUNKS.geo","Chunks.geo");
        int last_slash = str.lastIndexOf('/');
        if(-1!=last_slash)
        {
            geo_path = str.mid(0,last_slash);
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
