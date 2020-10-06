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
    mutable FileAccessRef m_fa;
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
        return FileAccess::exists(qPrintable(path));
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
    QSet<QString>                  s_missing_textures;
    std::unordered_map<uint32_t, Ref<Texture>> g_converted_textures;
}
SEGS::HTexture tryLoadTexture(const QString& fname)
{
    SEGS::RuntimeData& rd(getRuntimeData());
    QFileInfo tex_path(fname);
    QByteArray lookupstring = tex_path.baseName().toLower().toUtf8();
    QByteArray actualPath = rd.m_texture_paths.value(lookupstring);
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
            qDebug() << "Missing texture" << fname;
            s_missing_textures.insert(lookupstring);
        }
        return missing_tex_handle;
    }
    SEGS::HTexture& res(rd.m_loaded_textures[lookupstring]);
    if (g_converted_textures.find(res.idx) != g_converted_textures.end())
        return res; // we have an Urho3D texture already, nothing to do.

    QFile src_tex(actualPath);
    if (!src_tex.exists() || !src_tex.open(QFile::ReadOnly))
    {
        qWarning() << actualPath << " is not readable";
        return res;
    }
    TexFileHdr hdr;
    src_tex.read((char*)&hdr, sizeof(TexFileHdr));
    if (0 != memcmp(hdr.magic, "TX2", 3))
    {
        qWarning() << "Unrecognized texture format.";
        return res;
    }
    QString originalname = QString(src_tex.read(hdr.header_size - sizeof(TexFileHdr)));

    QDir converted_dir("./converted");
    QString converted_path(converted_dir.filePath(originalname));
    QFile tgt(converted_path);
    if (tgt.exists())
    {
        // a pre-converted texture file exists, load it instead
        g_converted_textures[res.idx] = dynamic_ref_cast<Texture>(gResourceManager().load(qPrintable(converted_path)));
        return res;
    }
    QByteArray data = src_tex.readAll();
    // save extracted texture into a local directory
    converted_dir.mkpath(QFileInfo(originalname).path());
    if (!tgt.open(QFile::WriteOnly))
    {
        qCritical() << "Cannot write:" << converted_path;
    }
    else
    {
        tgt.write(data);
        tgt.close();
    }
    auto entry = memnew(ImageTexture);
    g_converted_textures[res.idx] = entry;
    Image *img=memnew(Image);
    assert(false);
    return res;
}
std::vector<SEGS::HTexture> getModelTextures(std::vector<QString> &names)
{
    uint32_t name_count = std::max<uint32_t>(1,names.size());
    std::vector<SEGS::HTexture> res;
    res.reserve(name_count);
    SEGS::HTexture white_tex = tryLoadTexture("white.tga");

    for(size_t tex_idx=0; tex_idx < names.size(); ++tex_idx )
    {
        QFileInfo fi(names[tex_idx]);
        QString baseName = fi.completeBaseName();
        if(baseName!=names[tex_idx])
        {
            if(fi.fileName() == names[tex_idx])
                names[tex_idx] = baseName;
            else
                names[tex_idx] = fi.path()+"/"+baseName;
        }
        if ( names[tex_idx].contains("PORTAL",Qt::CaseInsensitive) )
            res.emplace_back(tryLoadTexture("invisible.tga"));
        else
            res.emplace_back(tryLoadTexture(names[tex_idx]));
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

Ref<Mesh> modelCreateObjectFromModel(SEGS::Model* model, std::vector<SEGS::HTexture>& textures)
{
    initLoadedModel([](const QString& v) -> SEGS::HTexture { return tryLoadTexture(v); }, model, textures);
    return Ref<Mesh>();
}


StringName CoHMeshLibrary::get_importer_name() const
{
    return "coh_modellib";
}

StringName CoHMeshLibrary::get_visible_name() const
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
static void flip_tri_order(Vector<int> &tris)
{
    for (size_t i = 0, count = tris.size(); i < count - 2; i += 3)
        eastl::swap(tris[i], tris[i + 1]);
}
static void fillArrayMesh(SEGS::Model* mdl, Ref<ArrayMesh> &tgt)
{
    SEGS::fillVBO(*mdl);
    SurfaceArrays arrs;

    arrs.m_position_data.assign((float*)mdl->vbo->pos.data(), (float*)mdl->vbo->pos.data() + mdl->vbo->pos.size()*3);
    arrs.m_normals.assign((Vector3*)mdl->vbo->norm.data(), (Vector3*)mdl->vbo->norm.data() + mdl->vbo->norm.size());
    arrs.m_indices.assign((int *)mdl->vbo->triangles.data(), (int *)mdl->vbo->triangles.data()+ mdl->vbo->triangles.size()*3);
    flip_tri_order(arrs.m_indices);
    arrs.m_uv_1.assign((Vector2*)mdl->vbo->uv1.data(), (Vector2*)mdl->vbo->uv1.data() + mdl->vbo->uv1.size());
    arrs.m_uv_2.assign((Vector2*)mdl->vbo->uv2.data(), (Vector2*)mdl->vbo->uv2.data() + mdl->vbo->uv2.size());
    for(const auto &bone : mdl->vbo->bone_indices)
    {
        arrs.m_bones.emplace_back(bone.first);
        arrs.m_bones.emplace_back(bone.second);
        for(int idx = 2; idx<RS::ARRAY_WEIGHTS_SIZE; ++idx)
        {
            arrs.m_bones.emplace_back(0);
        }
    }
    for (const auto& bone : mdl->vbo->bone_weights)
    {
        arrs.m_weights.emplace_back(bone[0]);
        arrs.m_weights.emplace_back(bone[1]);
        for (int idx = 2; idx < RS::ARRAY_WEIGHTS_SIZE; ++idx)
        {
            arrs.m_weights.emplace_back(0);
        }
    }

    arrs.m_vertices_2d = false;
    tgt->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES,eastl::move(arrs));


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
    for(SEGS::Model *mdl : gs.subs) {

        Ref<ArrayMesh> secondary = make_ref_counted<ArrayMesh>();
        String mdl_name = mdl->name.data();
        SEGS::toSafeModelName(mdl_name.data(),mdl_name.size());
        secondary->set_name(mdl_name);
        String mesh_name(secondary->get_name()+String(".mesh"));
        fillArrayMesh(mdl,secondary);
        created_models.push_back(base_path+"/"+mesh_name);
        gResourceManager().save(base_path+"/"+mesh_name, secondary);
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
    Ref<MeshLibrary> part_lib(build_mesh_library(p_source_file,*r_gen_files));

    return gResourceManager().save(String(p_save_path) + ".meshlib", part_lib);
}
