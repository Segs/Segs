/*************************************************************************/
/*  resource_importer_texture.cpp                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "resource_importer_geo.h"

#include "core/class_db.h"
#include "core/image.h"
#include "core/io/config_file.h"
#include "core/io/image_loader.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/dir_access.h"
#include "core/os/mutex.h"
#include "core/project_settings.h"
#include "core/resource/resource_manager.h"
#include "core/service_interfaces/CoreInterface.h"
#include "core/string_utils.h"
#include "editor/service_interfaces/EditorServiceInterface.h"

#include "scene/resources/primitive_meshes.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"
#include "scene/resources/material.h"

#include "Prefab.h"
#include "RuntimeData.h"
#include "Texture.h"
#include "SceneGraph.h"
#include "Common/Runtime/Model.h"
#include "GameData/trick_definitions.h"

#include "glm/gtx/matrix_decompose.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <scene/resources/mesh_library.h>

struct FileIOWrap : public QIODevice
{
    explicit FileIOWrap(FileAccess *fa)
        : m_fa(fa)
    {
    }
    bool isSequential() const override { return false; }
    /*qint64 pos() const
    {
        return m_fa->get_position();
    }*/
    qint64 size() const override
    {
        return m_fa->get_len();
    }
    bool seek(qint64 pos) override
    {
        QIODevice::seek(pos);
        m_fa->seek(pos);
        return this->pos()==pos;
    }
    qint64 bytesAvailable() const override
    {
        return m_fa->get_len()-pos();
    }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        return m_fa->get_buffer((uint8_t *)data,maxlen);
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        auto p = pos();
        m_fa->store_buffer((const uint8_t*)data, len);
        return pos() - p;
    }

public:
    mutable FileAccessRef<> m_fa;
};
struct SE_FSWrapper : public FSWrapper
{
    static Set<String> missing_files;
    SE_FSWrapper() {
        missing_files.clear();
    }
    QIODevice * open(const QString &path, bool read_only, bool text_only) override
    {
        FileAccess *wrap(FileAccess::open(qPrintable(path), read_only ? FileAccess::READ : FileAccess::READ_WRITE));
        if(!wrap) {

            missing_files.insert(qPrintable("res://"+path.mid(path.lastIndexOf("coh_data"))));
            return nullptr;
        }
        auto res = new FileIOWrap(wrap);
        res->open((read_only ? QIODevice::ReadOnly : QIODevice::ReadWrite)| (text_only ? QIODevice::Text : QIODevice::NotOpen));
        return res;
    }
    bool exists(const QString &path) override
    {
        return DirAccess::exists(qPrintable(path))||FileAccess::exists(qPrintable(path));
    }
    QStringList dir_entries(const QString &path) override
    {
        DirAccessRef da(DirAccess::open(qPrintable(path)));
        if(!da)
            return {}; //TODO: report missing directory?
        da->list_dir_begin();
        QStringList res;
        String item;
        while (!(item = da->get_next()).empty()) {

            if (item == "." || item == "..")
                continue;
            res.push_back(StringUtils::from_utf8(item));
        }
        return res;
    }
};
Set<String> SE_FSWrapper::missing_files;

Color fromGLM(glm::vec4 v) {
    return Color(v.r,v.g,v.b,v.a);
}

namespace
{
#pragma pack(push, 1)
    struct TexFileHdr
    {
        int     header_size;
        int     file_size;
        int     wdth;
        int     hght;
        int     flags;
        int     fade[2];
        uint8_t alpha;
        char    magic[3];
    };
#pragma pack(pop)
    HashSet<String> s_missing_textures;
    HashMap<uint32_t, Ref<Texture>> g_converted_textures;
}
void scan_for_textures(FSWrapper *fs_wrap,DirAccessRef &in) {
    in->list_dir_begin();
    Vector<String> dirs;
    String f;
    String cur_dir = StringUtils::replace(in->get_current_dir(),"\\", "/");

    while (!(f = in->get_next()).empty()) {
        if(in->current_is_hidden() || f=="." || f=="..")
            continue;
        if (in->current_is_dir()) {
            DirAccessRef da(DirAccess::open(cur_dir+"/"+f));
            scan_for_textures(fs_wrap,da);
        }
        else {
            if(f.ends_with(".texture")) {
                SEGS::RuntimeData& rd(getRuntimeData());
                QFileInfo tex_path(f.c_str());
                QByteArray lookupstring = tex_path.baseName().toLower().toUtf8();

                rd.m_texture_paths[lookupstring] = (cur_dir + "/" + f).c_str();
                SEGS::loadTexHeader(fs_wrap,rd.m_texture_paths[lookupstring]);
            }
        }
    }
    in->list_dir_end();

}
void build_path_map() {
    SE_FSWrapper se_wrap;

    SEGS::RuntimeData& rd(getRuntimeData());
    if(!rd.m_texture_paths.empty()) // don't repeat the scan.
        return;

    DirAccessRef da(DirAccess::open("res://coh_data/texture_library"));
    if(!da) {
        return;
    }
    scan_for_textures(&se_wrap,da);
}

SEGS::HTexture tryLoadTexture(const String &fname)
{
    SEGS::RuntimeData& rd(getRuntimeData());
    QFileInfo tex_path(fname.c_str());
    String lookupstring = tex_path.baseName().toLower().toUtf8().data();
    QByteArray actualPath   = rd.m_texture_paths.value(lookupstring.c_str());
    static SEGS::HTexture missing_tex_handle;
    if (actualPath.isEmpty())
    {
        if (!missing_tex_handle)
        {
            missing_tex_handle = SEGS::TextureStorage::instance().create();
            g_converted_textures[missing_tex_handle.idx] = dynamic_ref_cast< Texture >(gResourceManager().load("Textures/Missing.dds"));

        }
        if (!s_missing_textures.contains(lookupstring))
        {
            getCoreInterface()->reportError("Missing texture" + fname,"",FUNCTION_STR, __FILE__, __LINE__);
            s_missing_textures.insert(lookupstring);
        }
        return missing_tex_handle;
    }
    SEGS::HTexture &res(rd.m_loaded_textures[lookupstring.c_str()]);
    if (g_converted_textures.find(res.idx) != g_converted_textures.end())
        return res; // we have an Urho3D texture already, nothing to do.

    g_converted_textures[res.idx] = dynamic_ref_cast<Texture>(gResourceManager().load(actualPath.data()));
        return res;
    }

Vector<SEGS::HTexture> getModelTextures(std::vector<QByteArray> &names)
    {
    uint32_t name_count = eastl::max<uint32_t>(1,names.size());
    Vector<SEGS::HTexture> res;
    res.reserve(name_count);
    SEGS::HTexture white_tex = tryLoadTexture("white.tga");

    for(size_t tex_idx=0; tex_idx < names.size(); ++tex_idx )
    {
        QFileInfo fi(names[tex_idx]);
        QByteArray baseName = fi.completeBaseName().toUtf8();
        if(baseName!=names[tex_idx])
        {
            if(fi.fileName() == names[tex_idx])
                names[tex_idx] = baseName;
            else
                names[tex_idx] = fi.path().toUtf8()+"/"+baseName;
        }
        if ( names[tex_idx].toUpper().contains("PORTAL") )
            res.emplace_back(tryLoadTexture("invisible.tga"));
        else
            res.emplace_back(tryLoadTexture(names[tex_idx].data()));
        // replace missing texture with white
        // TODO: make missing textures much more visible ( high contrast + text ? )
        if ( g_converted_textures.end()==g_converted_textures.find(res[tex_idx].idx) )
        {
            res[tex_idx] = white_tex;
        }
    }
    if (names.empty())
        res.emplace_back(white_tex);
    return res;
}

void initLoadedModel(SEGS::Model *model)
{
    using namespace SEGS;
    model->blend_mode = SEGS::CoHBlendMode::MULTIPLY_REG;
    bool isgeo=false;
    if(model->name.toUpper().startsWith("GEO_"))
    {
        model->flags |= SEGS::OBJ_DRAW_AS_ENT;
        isgeo = true;
        if(model->name.toLower().contains("eyes") )
        {
            if(!model->trck_node)
                model->trck_node = new ModelModifiers;
            model->trck_node->_TrickFlags |= DoubleSided;
        }
    }
    assert(model->num_textures==model->texture_bind_info.size());

    if( model->trck_node && model->trck_node->info)
    {
        model->flags |= model->trck_node->info->ObjFlags;
    }

    if( model->blend_mode == CoHBlendMode::COLORBLEND_DUAL || model->blend_mode == CoHBlendMode::BUMPMAP_COLORBLEND_DUAL )
    {
        if( !model->trck_node )
            model->trck_node = new ModelModifiers;
        model->trck_node->_TrickFlags |= SetColor;
    }

    if( model->blend_mode == CoHBlendMode::ADDGLOW )
    {
        if( !model->trck_node )
            model->trck_node = new ModelModifiers;
        model->trck_node->_TrickFlags |= SetColor | NightLight;
    }

    if( !model->packed_data.norms.uncomp_size ) // no normals
        model->flags |= OBJ_FULLBRIGHT; // only ambient light
    if( model->trck_node  && model->trck_node->_TrickFlags & Additive )
        model->flags |= OBJ_ALPHASORT; // alpha pass
    if( model->flags & OBJ_FORCEOPAQUE ) // force opaque
        model->flags &= ~OBJ_ALPHASORT;

    if( model->trck_node && model->trck_node->info)
    {
        if( model->trck_node->info->blend_mode )
            model->blend_mode = CoHBlendMode(model->trck_node->info->blend_mode);
    }
}
static Vector3 fromGLM(glm::vec3 v) {
    return {v.x,v.y,v.z};
}
static Vector2 fromGLM(glm::vec2 v) {
    return {v.x,v.y};
}
static int reindex(Vector<int> &reindex,int vert_idx,int &top_cnt) {
    if(!reindex[vert_idx])
        reindex[vert_idx] = top_cnt++;
    return reindex[vert_idx]-1;
}
static SurfaceArrays from_coh_vbo(std::unique_ptr<SEGS::VBOPointers> &vbo,int offset,int tri_count) {
    SurfaceArrays arrs;

    arrs.m_indices.reserve(vbo->triangles.size());
    arrs.m_position_data.reserve(vbo->triangles.size()*3);
    arrs.m_normals.reserve(vbo->triangles.size());
    arrs.m_uv_1.reserve(vbo->triangles.size());
    arrs.m_uv_2.reserve(vbo->triangles.size());
    Vector<int> reindex_arr(vbo->triangles.size(),0);
    int *tri_data=vbo->triangles.data()+offset;
    Vector<int> src_triangles(tri_data,tri_data+3*tri_count);
    int last_idx=1;
    int new_idx[3];
    bool known[3];
    for (size_t tri_index = 0, count = src_triangles.size(); tri_index < count - 2; tri_index += 3) {
        //reverse winding 0,1,2 -> 1,0,2
        for(int i=0; i<3; ++i) {
            known[i] = reindex_arr[src_triangles[tri_index+i]]!=0;
            new_idx[i] = reindex(reindex_arr,src_triangles[tri_index+i],last_idx);
        }

        // inserting indices
        arrs.m_indices.insert(arrs.m_indices.end(),new_idx,new_idx+3);

        for(int i=0; i<3; ++i) {
            int idx = src_triangles[tri_index+i];
            if(!known[i])
                arrs.m_position_data.insert(arrs.m_position_data.end(),(float *)&vbo->pos[idx],(float *)&vbo->pos[idx]+3);
        }

        if(!vbo->norm.empty())
            for(int i=0; i<3; ++i) {
                int idx = src_triangles[tri_index+i];
                if(!known[i])
                    arrs.m_normals.emplace_back(fromGLM(vbo->norm[idx]));
            }
        if(!vbo->uv1.empty()) {
            for(int i=0; i<3; ++i) {
                int idx = src_triangles[tri_index+i];
                if(!known[i])
                    arrs.m_uv_1.emplace_back(fromGLM(vbo->uv1[idx]));
            }
        }

        if(!vbo->uv2.empty()) {
            for(int i=0; i<3; ++i) {
                int idx = src_triangles[tri_index+i];
                if(!known[i])
                    arrs.m_uv_2.emplace_back(fromGLM(vbo->uv2[idx]));
            }
        }
        if(!vbo->bone_indices.empty() && vbo->bone_weights.empty()) {
            for(int i=0; i<3; ++i) {
                int idx = src_triangles[tri_index+i];
                if(known[i])
                    continue;

                auto bone=vbo->bone_indices[idx];
                arrs.m_bones.emplace_back(bone.first);
                arrs.m_bones.emplace_back(bone.second);
                auto bonew=vbo->bone_weights[idx];
                arrs.m_weights.emplace_back(bonew[0]);
                arrs.m_weights.emplace_back(bonew[1]);
                for (int idx = 2; idx < RS::ARRAY_WEIGHTS_SIZE; ++idx)
                {
                    arrs.m_weights.emplace_back(0);
                    arrs.m_weights.emplace_back(0);
                }
            }
        }
    }
    arrs.m_vertices_2d = false;
    return arrs;
}
static Ref<Material> convert_material(int idx,SEGS::Model* model,Vector<SEGS::HTexture>& textures,String &desc) {
    using namespace SEGS;
    SpatialMaterial * mat = memnew(SpatialMaterial);
    SEGS::HTexture tex = textures[idx];

    model->blend_mode = SEGS::CoHBlendMode::MULTIPLY_REG;
    bool isgeo=false;
    if(model->name.toUpper().startsWith("GEO_"))
    {
        model->flags |= SEGS::OBJ_DRAW_AS_ENT;
        isgeo = true;
    }
    if(model->trck_node && model->trck_node->_TrickFlags & DoubleSided)
        mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);
    else
        mat->set_cull_mode(SpatialMaterial::CULL_FRONT);

    SEGS::TextureWrapper &base_tex(tex.get());
    if(!isgeo && base_tex.info)
    {
        auto loc_tex = tex;
        auto blend = CoHBlendMode(base_tex.info->BlendType);
        if(blend == CoHBlendMode::ADDGLOW || blend == CoHBlendMode::COLORBLEND_DUAL || blend == CoHBlendMode::ALPHADETAIL) {
            loc_tex = tryLoadTexture(base_tex.detailname.data());
            mat->set_texture(SpatialMaterial::TEXTURE_DETAIL_ALBEDO,g_converted_textures[loc_tex.idx]);
        }
        if(loc_tex && loc_tex->flags & TextureWrapper::ALPHA)
            mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
    }
    if( base_tex.flags & TextureWrapper::DUAL )
    {
        model->flags |= OBJ_DUALTEXTURE;
        if( base_tex.BlendType != CoHBlendMode::MULTIPLY )
            model->blend_mode = base_tex.BlendType;
    }

    if( !base_tex.bumpmap.isEmpty() )
    {
        HTexture wrap = tryLoadTexture(base_tex.bumpmap.data());
        if( wrap->flags & TextureWrapper::BUMPMAP )
        {
            mat->set_texture(SpatialMaterial::TEXTURE_NORMAL,g_converted_textures[wrap.idx]);
            model->flags |= OBJ_BUMPMAP;
            model->blend_mode = (model->blend_mode == CoHBlendMode::COLORBLEND_DUAL) ?
                        CoHBlendMode::BUMPMAP_COLORBLEND_DUAL : CoHBlendMode::BUMPMAP_MULTIPLY;
        }
        if( base_tex.flags & TextureWrapper::CUBEMAPFACE || (wrap->flags & TextureWrapper::CUBEMAPFACE) ) {
            model->flags |= OBJ_CUBEMAP;
        }
    }
    mat->set_texture(SpatialMaterial::TEXTURE_ALBEDO,g_converted_textures[tex.idx]);
    return Ref<Material>(mat);
}
Ref<ArrayMesh> modelCreateObjectFromModel(SEGS::Model* model, Vector<SEGS::HTexture>& textures)
{
    Ref<ArrayMesh> secondary = make_ref_counted<ArrayMesh>();
    String mdl_name = model->name.data();
    SEGS::toSafeModelName(mdl_name.data(),mdl_name.size());
    secondary->set_name(mdl_name);

    initLoadedModel(model);
    if ( model->name.toUpper().startsWith("GEO_") )
    {
        model->flags |= SEGS::OBJ_DRAW_AS_ENT;
        if ( model->name.toLower().contains("eyes") )
        {
            if ( !model->trck_node )
                model->trck_node = new ModelModifiers;
            model->trck_node->_TrickFlags |= DoubleSided;
        }
    }
    SEGS::fillVBO(*model);
    std::unique_ptr<SEGS::VBOPointers> &vbo(model->vbo);

    unsigned geom_count = model->texture_bind_info.size();
    unsigned face_offset=0;
    String collected_desc;

    int *tris = (int *)vbo->triangles.data();
    //flip_tri_order(Span<int>(tris,tris+vbo->triangles.size()*3));

    for(unsigned i=0; i<geom_count; ++i)
    {
        const SEGS::TextureBind &tbind(model->texture_bind_info[i]);

        SurfaceArrays arrs=from_coh_vbo(vbo,face_offset,tbind.tri_count);
        secondary->add_surface_from_arrays(ArrayMesh::PRIMITIVE_TRIANGLES,eastl::move(arrs));
        secondary->surface_set_material(i,convert_material(tbind.tex_idx,model,textures,collected_desc));

        face_offset+=tbind.tri_count*3;
    }

    return secondary;
}


const char *CoHMeshLibrary::get_importer_name() const
{
    return "coh_modellib";
}

const char *CoHMeshLibrary::get_visible_name() const
{
    return "CoH Model Library";
}

void CoHMeshLibrary::get_recognized_extensions(Vector<String> &p_extensions) const
{
    // CoH scene graph files have this extension.
    p_extensions.emplace_back("geo");
}

bool CoHMeshLibrary::can_import(StringView path) const
{
    return path.contains("coh_data/object_library/");
}

StringName CoHMeshLibrary::get_save_extension() const
{
    return "meshlib";
}

StringName CoHMeshLibrary::get_resource_type() const
{
    return "MeshLibrary";
}


StringName CoHMeshLibrary::get_preset_name(int p_idx) const {

    return StringName();
}

bool CoHMeshLibrary::get_option_visibility(const StringName &p_option, const HashMap<StringName, Variant> &p_options) const
{
    return true;
}


static Ref<MeshLibrary> build_mesh_library(StringView path,Vector<String> &created_models)
{
    Ref<MeshLibrary> res = make_ref_counted<MeshLibrary>();
    SEGS::GeoSet gs;
    HashSet<String> exported_scene_roots;
    SE_FSWrapper se_wrap;
    QIODevice *src = se_wrap.open(QString::fromUtf8(path.data(),path.size()),true,false);
    String base_path = PathUtils::get_base_dir(path);
    geosetLoadHeader(src,&gs);
    geosetLoadData(src,&gs);
    int idx=0;
    Vector<SEGS::HTexture> model_textures = getModelTextures(gs.tex_names);
    for(SEGS::Model *mdl : gs.subs) {
        Ref<ArrayMesh> secondary = modelCreateObjectFromModel(mdl, model_textures);
        String mesh_name(secondary->get_name()+String(".mesh"));

//        String mesh_name2(secondary->get_name()+String(".tres"));
        created_models.push_back(base_path+"/"+mesh_name);
        gResourceManager().save(base_path+"/"+mesh_name, secondary,ResourceManager::FLAG_COMPRESS);
//        gResourceManager().save(base_path+"/"+mesh_name2, secondary);
        res->create_item(idx);
        res->set_item_mesh(idx,secondary);
        res->set_item_name(idx,secondary->get_name());
        idx++;
    }

    return eastl::move(res);
}

Error CoHMeshLibrary::import(StringView p_source_file, StringView p_save_path,
                             const HashMap<StringName, Variant> &p_options, Vector<String> &r_missing_deps,
                             Vector<String> *r_platform_variants, Vector<String> *r_gen_files, Variant *r_metadata)
{
    SE_FSWrapper se_wrap;

    // Check if the selected file is correctly located in the hierarchy, just to make sure.
    auto idx = StringUtils::find_last(p_source_file, "coh_data/object_library/");
    if (String::npos == idx)
    {
        PLUG_FAIL_V_MSG(ERR_CANT_OPEN, "The given source file is not located in coh_data/object_library/ folder.");
    }

    String fs_path(p_source_file);
    fs_path.replace("res:/", ProjectSettings::get_singleton()->get_resource_path());

    auto& rd(getRuntimeData());
    if (!rd.m_ready)
    {
        // we expect coh data to live under res://coh_data
        String basepath = ProjectSettings::get_singleton()->get_resource_path() + "/coh_data/";

        if (!rd.prepare(&se_wrap, basepath.c_str()))
        {
            PLUG_FAIL_V_MSG(ERR_FILE_MISSING_DEPENDENCIES, "The required bin files are missing ?");
        }
    }
    assert(r_gen_files);

    build_path_map();

    Ref<MeshLibrary> part_lib(build_mesh_library(p_source_file,*r_gen_files));

    return gResourceManager().save(String(p_save_path) + ".meshlib", part_lib);
}
