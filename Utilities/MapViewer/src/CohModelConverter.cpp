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

#include "CohModelConverter.h"

#include "CoHModelLoader.h"
#include "CoHSceneConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "Common/Runtime/Model.h"
#include "Common/Runtime/Texture.h"
#include "Common/Runtime/Prefab.h"

#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
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

// Start of anonymous namespace
namespace {
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

/// this map is used too lookup converted models by the SEGS::Model pointer
std::unordered_map<SEGS::Model *,Urho3D::SharedPtr<Urho3D::Model>> s_coh_model_to_engine_model;
std::unordered_map<SEGS::Model *,Urho3D::SharedPtr<Urho3D::StaticModel>> s_coh_model_to_static_model;

void reportUnhandled(const QString &message)
{
    static QSet<QString> already_reported;
    if(already_reported.contains(message))
        return;
    qDebug() << message;
    already_reported.insert(message);
}

SharedPtr<Urho3D::Model> modelCreateObjectFromModel(Urho3D::Context *ctx,SEGS::Model *model,std::vector<SEGS::HTexture> &textures)
{
    initLoadedModel([ctx](const QString &v) -> SEGS::HTexture { return tryLoadTexture(ctx, v); }, model, textures);
    SEGS::fillVBO(*model);
    std::unique_ptr<SEGS::VBOPointers> &vbo(model->vbo);
    vbo->assigned_textures.reserve(textures.size());
    for(SEGS::TextureBind tbind : model->texture_bind_info)
    {
        vbo->assigned_textures.emplace_back(textures[tbind.tex_idx]);
    }

    SharedPtr<Urho3D::Model> fromScratchModel(new Urho3D::Model(ctx));
    SharedPtr<VertexBuffer> vb(new VertexBuffer(ctx));
    SharedPtr<IndexBuffer> ib(new IndexBuffer(ctx));
    std::vector<VertexElement> vertex_elements;
    vertex_elements.emplace_back(TYPE_VECTOR3, SEM_POSITION);
    if(model->packed_data.norms.uncomp_size)
        vertex_elements.emplace_back(TYPE_VECTOR3, SEM_NORMAL);
    if(!vbo->uv1.empty())
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD,0);
    if(!vbo->uv2.empty())
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD,1);
    if (vbo->needs_tangents)
        vertex_elements.emplace_back(TYPE_VECTOR4, SEM_TANGENT);

    float *combined = combineBuffers(*vbo,model);
    vb->SetShadowed(true);
    vb->SetSize(model->vertex_count, vertex_elements);
    vb->SetData(combined);
    delete [] combined;
    ib->SetShadowed(true);
    ib->SetSize(model->model_tri_count*3, true);
    ib->SetData(vbo->triangles.data());
    fromScratchModel->SetVertexBuffers({vb},{},{});
    fromScratchModel->SetIndexBuffers({ib});

    BoundingBox bbox(toUrho(model->box.m_min),toUrho(model->box.m_max));
    unsigned geom_count = model->texture_bind_info.size();
    fromScratchModel->SetNumGeometries(geom_count);
    unsigned face_offset=0;
    for(unsigned i=0; i<geom_count; ++i)
    {
        const SEGS::TextureBind &tbind(model->texture_bind_info[i]);
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
    return fromScratchModel;
}

void createEngineModelsFromPrefabSet(Urho3D::Context *ctx,SEGS::GeoSet *geoset)
{
    std::vector<SEGS::HTexture> model_textures = getModelTextures(ctx,geoset->tex_names);
    for(SEGS::Model * model : geoset->subs)
    {
        s_coh_model_to_engine_model[model] = modelCreateObjectFromModel(ctx,model, model_textures);
    }
}

Urho3D::Model *buildModel(Urho3D::Context *ctx,SEGS::Model *mdl)
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
        auto lf3d_model=cache->GetResource<Urho3D::Model>(cache_path);
        if(lf3d_model)
        {
            auto textures = getModelTextures(ctx,mdl->geoset->tex_names);
            initLoadedModel([ctx](const QString &v) -> SEGS::HTexture { return tryLoadTexture(ctx, v); },mdl,textures);
            return lf3d_model;
        }
    }
    QFile fl(basepath+mdl->geoset->geopath);
    if(!fl.exists() || !fl.open(QFile::ReadOnly) || s_coh_model_to_engine_model.size()>5000)
    {
        if(s_coh_model_to_engine_model.size()<=3) // this will report first few missing geometry files
        {
            qDebug() << "Missing geo file" << basepath+mdl->geoset->geopath;
        }
        return nullptr;
    }
    if(!mdl->geoset->data_loaded) {
        geosetLoadData(fl,mdl->geoset);
        if (!mdl->geoset->subs.empty())
            createEngineModelsFromPrefabSet(ctx,mdl->geoset);

    }
    QDir modeldir("converted/Models");
    modeldir.mkpath(parts.join('/'));
    auto res_static=s_coh_model_to_engine_model[mdl];
    File model_res(ctx,cache_path,FILE_WRITE);
    res_static->Save(model_res);
    res_static->SetName("Models/"+parts.join('/')+"/"+mdl->name+".mdl");
    cache->AddManualResource(res_static);
    return res_static;
}

void convertMaterial(Urho3D::Context *ctx,CoHModel *mdl,StaticModel* boxObject)
{
    using namespace SEGS;
    static std::unordered_set<CoHModel *> already_converted;
    if (already_converted.find(mdl) == already_converted.end())
    {
        already_converted.insert(mdl);
    }
    else
        assert(false);
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
            onlyColor = Color(model_trick->TintColor0.rgba.r/255.0f,
                              model_trick->TintColor0.rgba.g/255.0f,
                              model_trick->TintColor0.rgba.b/255.0f,
                              model_trick->TintColor0.rgba.a/255.0f);
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
    HTexture whitetex = tryLoadTexture(ctx,"white.tga");
    QStringList vertex_defines;
    QStringList pixel_defines;
    SharedPtr<Material> preconverted;
    if(cache->Exists("./converted/Materials/"+model_base_name+"_mtl.xml"))
        preconverted = cache->GetResource<Material>("./converted/Materials/"+model_base_name+"_mtl.xml");
    bool noLightAngle= mdl->flags & SEGS::OBJ_NOLIGHTANGLE;
    if(!preconverted) {
        if(mdl->flags&SEGS::OBJ_TREE)
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
        pixel_defines << "COH_MULTIPLY";
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
    bool is_single_mat = mdl->texture_bind_info.size() == 1;
    for(auto & texbind : mdl->texture_bind_info)
    {
        const QString &texname(mdl->geoset->tex_names[texbind.tex_idx]);
        HTexture tex = tryLoadTexture(ctx,texname);
        auto iter = g_converted_textures.find(tex.idx);
        geomidx++;
        if(iter==g_converted_textures.end())
            continue;
        QString mat_name = QString("Materials/%1%2_mtl.xml").arg(model_base_name).arg(geomidx-1);
        Urho3D::Material *cached_mat = cache->GetResource<Material>(mat_name,false);
        Urho3D::SharedPtr<Urho3D::Texture> &engine_tex(iter->second);
        if(cached_mat && cached_mat->GetTexture(TU_DIFFUSE)==engine_tex)
        {
            boxObject->SetMaterial(geomidx-1,cached_mat);
            continue;
        }
        result = is_single_mat ? preconverted : preconverted->Clone();
        result->SetTexture(TU_DIFFUSE,engine_tex);
        HTexture custom1 = whitetex;
        HTexture bump_tex = {};
        if(tex->info)
        {
            if(!tex->info->Blend.isEmpty())
                custom1 = tryLoadTexture(ctx,tex->info->Blend);
            if(!tex->info->BumpMap.isEmpty())
                bump_tex = tryLoadTexture(ctx,tex->info->BumpMap);
        }
        result->SetTexture(TU_CUSTOM1,g_converted_textures[custom1.idx]);
        if(bump_tex)
            result->SetTexture(TU_NORMAL,g_converted_textures[bump_tex.idx]);

        QDir modeldir("converted/");
        bool created = modeldir.mkpath("Materials");
        assert(created);
        QString cache_path="./converted/"+mat_name;
        File mat_res(ctx, cache_path, FILE_WRITE);
        result->SetName(mat_name);
        result->Save(mat_res);
        cache->AddManualResource(result);
        boxObject->SetMaterial(geomidx-1,cache->GetResource<Material>(mat_name));
    }

}

void copyStaticModel(Urho3D::StaticModel *src, Urho3D::StaticModel *tgt)
{
    tgt->SetModel(src->GetModel());
    int geoms = src->GetNumGeometries();
    for(int i=0; i<geoms;++i)
    {
        tgt->SetMaterial(i, src->GetMaterial(i));
    }
}
} // end of anonymus namespace

Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, SEGS::SceneNode *node, int opt)
{
    SEGS::Model *mdl = node->m_model;
    Urho3D::StaticModel * converted=nullptr;
    auto loc = s_coh_model_to_static_model.find(mdl);
    if (loc != s_coh_model_to_static_model.end())
        converted = loc->second.Get();

    if (mdl && converted)
    {
        float per_node_draw_distance = node->lod_far + node->lod_far_fade;
        StaticModel* boxObject = tgtnode->CreateComponent<StaticModel>();
        copyStaticModel(converted, boxObject);
        boxObject->SetDrawDistance(per_node_draw_distance);
        return boxObject;
    }

    ModelModifiers *model_trick = mdl->trck_node;
    if (model_trick)
    {
        if (opt != CONVERT_EDITOR_MARKERS && model_trick->isFlag(NoDraw))
        {
            //qDebug() << mdl->name << "Set as no draw";
            return nullptr;
        }
        if (opt != CONVERT_EDITOR_MARKERS && model_trick->isFlag(EditorVisible))
        {
            //qDebug() << mdl->name << "Set as editor model";
            return nullptr;
        }
        if (model_trick && model_trick->isFlag(CastShadow))
        {
            //qDebug() << "Not converting shadow models"<<mdl->name;
            return nullptr;
        }
        if (model_trick && model_trick->isFlag(ParticleSys))
        {
            qDebug() << "Not converting particle sys:" << mdl->name;
            return nullptr;
        }
    }
    Urho3D::Model *modelptr = buildModel(ctx, mdl);
    if(!modelptr)
        return nullptr;
    StaticModel* boxObject = tgtnode->CreateComponent<StaticModel>();

    boxObject->SetDrawDistance(node->lod_far+node->lod_far_fade);
    boxObject->SetModel(modelptr);
    convertMaterial(ctx,mdl,boxObject);
    s_coh_model_to_static_model[mdl] = boxObject;
    return boxObject;
}

//! @}
