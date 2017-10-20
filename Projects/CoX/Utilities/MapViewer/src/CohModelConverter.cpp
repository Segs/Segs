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
/// this map is used too lookup converted models by the ConvertedModel pointer
std::unordered_map<ConvertedModel *,Urho3D::SharedPtr<Model>> s_coh_model_to_renderable;
QHash<QString,ConvertedGeoSet *> s_name_to_geoset;

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
void initLoadedModel(Urho3D::Context *ctx,ConvertedModel *model,const std::vector<TextureWrapper> &textures)
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
}

void reportUnhandled(const QString &message)
{
    static QSet<QString> already_reported;
    if(already_reported.contains(message))
        return;
    qDebug() << message;
    already_reported.insert(message);
}
void modelCreateObjectFromModel(Urho3D::Context *ctx,ConvertedModel *model,std::vector<TextureWrapper> &textures)
{
    initLoadedModel(ctx,model,textures);
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
            initLoadedModel(ctx,mdl,v2);
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
        geosetLoadData(ctx,fl,mdl->geoset);
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
Urho3D::StaticModel *convertedModelToLutefisk(Urho3D::Context *ctx, Urho3D::Node *tgtnode, CoHNode *node, int opt)
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
