#include "CohModelConverter.h"

#include "CoHModelLoader.h"
#include "CoHSceneConverter.h"
#include "CohTextureConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"

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


AllTricks_Data s_tricks_store;
/// this map is used too lookup converted models by the CoHModel pointer
std::unordered_map<CoHModel *,Urho3D::SharedPtr<Model>> s_coh_model_to_renderable;
QHash<QString,ConvertedGeoSet *> s_name_to_geoset;

void reportUnhandled(const QString &message)
{
    static QSet<QString> already_reported;
    if(already_reported.contains(message))
        return;
    qDebug() << message;
    already_reported.insert(message);
}
void modelCreateObjectFromModel(Urho3D::Context *ctx,CoHModel *model,std::vector<TextureWrapper> &textures)
{
    initLoadedModel([ctx](const QString &v) -> TextureWrapper { return tryLoadTexture(ctx, v); }, model, textures);
    std::unique_ptr<VBOPointers> vbo(getVBO(*model));
    vbo->assigned_textures.reserve(textures.size());
    for(TextureBind tbind : model->texture_bind_info)
    {
        TextureWrapper &texb(textures[tbind.tex_idx]);
        vbo->assigned_textures.emplace_back(texb);
    }

    SharedPtr<Urho3D::Model> fromScratchModel(new Urho3D::Model(ctx));
    SharedPtr<VertexBuffer> vb(new VertexBuffer(ctx));
    SharedPtr<IndexBuffer> ib(new IndexBuffer(ctx));
    std::vector<VertexElement> vertex_elements;
    vertex_elements.emplace_back(TYPE_VECTOR3, SEM_POSITION);
    if(model->packed_data.norms.uncomp_size)
        vertex_elements.emplace_back(TYPE_VECTOR3, SEM_NORMAL);
    if(!vbo->uv1.empty())
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD);
    if(!vbo->uv2.empty())
        vertex_elements.emplace_back(TYPE_VECTOR2, SEM_TEXCOORD);
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

    BoundingBox bbox(toUrho(model->m_min),toUrho(model->m_max));
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
    for(CoHModel * model : geoset->subs)
    {
        modelCreateObjectFromModel(ctx,model, v2);
    }
}
Urho3D::Model *buildModel(Urho3D::Context *ctx,CoHModel *mdl)
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
            initLoadedModel([ctx](const QString &v) -> TextureWrapper { return tryLoadTexture(ctx, v); },mdl,v2);
            s_coh_model_to_renderable[mdl] = lf3d_model;
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
        geosetLoadData(fl,mdl->geoset);
        if (!mdl->geoset->subs.empty())
            addModelData(ctx,mdl->geoset);

    }
    QDir modeldir("converted/Models");
    modeldir.mkpath(parts.join('/'));
    auto res=s_coh_model_to_renderable[mdl];
    File model_res(ctx,cache_path,FILE_WRITE);
    res->Save(model_res);
    return res;
}

void convertMaterial(Urho3D::Context *ctx,CoHModel *mdl,StaticModel* boxObject)
{
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
    bool is_single_mat = mdl->texture_bind_info.size() == 1;
    for(auto & texbind : mdl->texture_bind_info)
    {
        const QString &texname(mdl->geoset->tex_names[texbind.tex_idx]);
        TextureWrapper tex = tryLoadTexture(ctx,texname);
        if(tex.base) 
        {
            result = is_single_mat ? preconverted : preconverted->Clone();
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
            QDir modeldir("converted/");
            assert(modeldir.mkpath("Materials"));
            QString cache_path("./converted/Materials/" + model_base_name + QString::number(geomidx)+"_mtl.xml");
            auto res = s_coh_model_to_renderable[mdl];
            File model_res(ctx, cache_path, FILE_WRITE);
            result->Save(model_res);
        }
        geomidx++;
    }

}
void copyStaticModel(const Urho3D::StaticModel *src, Urho3D::StaticModel *tgt)
{
    tgt->SetModel(src->GetModel());
    int geoms = src->GetNumGeometries();
    for(int i=0; i<geoms;++i)
    {
        tgt->SetMaterial(i, src->GetMaterial(i));
    }
}
} // end of anonymus namespace
Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, CoHNode *node, int opt)
{
    CoHModel *mdl = node->model;
    if (mdl&&mdl->converted_model) 
    {
        float per_node_draw_distance = node->lod_far + node->lod_far_fade;
        //if (mdl->converted_model->GetDrawDistance() == per_node_draw_distance) // same draw distance as default, nothing to do
            return mdl->converted_model;
        StaticModel* boxObject = tgtnode->CreateComponent<StaticModel>();
        copyStaticModel(mdl->converted_model, boxObject);
        boxObject->SetDrawDistance(node->lod_far + node->lod_far_fade);
        return boxObject;
    }
    Model * modelptr=nullptr;
    auto loc = s_coh_model_to_renderable.find(mdl);
    if (loc != s_coh_model_to_renderable.end())
        modelptr = loc->second.Get();
    if (modelptr == nullptr) 
    {
        ModelModifiers *model_trick = mdl->trck_node;
        if (model_trick) {
            if (model_trick->isFlag(NoDraw))
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
        modelptr = buildModel(ctx, mdl);
        modelptr->SetName(mdl->name);
    }
    if(!modelptr)
        return nullptr;
    StaticModel* boxObject = tgtnode->CreateComponent<StaticModel>();
    //boxObject->SetDrawDistance(node->lod_far+node->lod_far_fade);
    boxObject->SetModel(modelptr);
    convertMaterial(ctx,mdl,boxObject);
    mdl->converted_model = boxObject;
    return boxObject;
}
