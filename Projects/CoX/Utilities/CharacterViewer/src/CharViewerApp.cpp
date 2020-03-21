/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CharViewer Projects/CoX/Utilities/CharViewer
 * @{
 */

#include "CharViewerApp.h"
#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "CohTextureConverter.h"
#include "Common/GameData/GameDataStore.h"
#include "Common/GameData/anim_definitions.h"
#include "Common/GameData/bodypart_definitions.h"
#include "Common/GameData/npc_definitions.h"
#include "Common/GameData/seq_definitions.h"
#include "Common/Runtime/Animation.h"
#include "Common/Runtime/AnimationEngine.h"
#include "Common/Runtime/Prefab.h"
#include "Common/Runtime/RuntimeData.h"
#include "Common/Runtime/SceneGraph.h"
#include "Common/Runtime/Sequencer.h"
#include "Common/Runtime/Texture.h"
#include "GameData/anim_definitions.h"
#include "SideWindow.h"

#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Core/CoreEvents.h>
#include <Lutefisk3D/Engine/Engine.h>
#include <Lutefisk3D/Engine/EngineDefs.h>
#include <Lutefisk3D/Graphics/AnimatedModel.h>
#include <Lutefisk3D/Graphics/Animation.h>
#include <Lutefisk3D/Graphics/AnimationController.h>
#include <Lutefisk3D/Graphics/Camera.h>
#include <Lutefisk3D/Graphics/DebugRenderer.h>
#include <Lutefisk3D/Graphics/Light.h>
#include <Lutefisk3D/Graphics/Material.h>
#include <Lutefisk3D/Graphics/Model.h>
#include <Lutefisk3D/Graphics/Octree.h>
#include <Lutefisk3D/Graphics/Renderer.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Graphics/Zone.h>
#include <Lutefisk3D/IO/File.h>
#include <Lutefisk3D/Input/Input.h>
#include <Lutefisk3D/Input/InputEvents.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/Scene/Scene.h>
#include <Lutefisk3D/SystemUI/Console.h>
#include <Lutefisk3D/SystemUI/DebugHud.h>
#include <Lutefisk3D/UI/BorderImage.h>
#include <Lutefisk3D/UI/Font.h>
#include <Lutefisk3D/UI/Text3D.h>
#include <Lutefisk3D/UI/UI.h>

#include <QSettings>
#include <QtCore/QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

using namespace Urho3D;
namespace
{
Urho3D::Vector3 fromGLM(glm::vec3 v)
{
    return {v.x, v.y, v.z};
}
Urho3D::Quaternion fromGLM(glm::quat v)
{
    return Urho3D::Quaternion(v.w, v.x, v.y, v.z);
}
} // end of anonymous namespace

struct LF3CharacterNode : public SEGS::SceneTreeNode
{
    SharedPtr<Node>      m_node;
    Skeleton *           m_skeleton           = nullptr;
    AnimationController *m_anim_controller    = nullptr;
    AnimationState *     m_anim_state         = nullptr;
    SEGS::RenderingData *m_rendering_data     = nullptr;
    StaticModel *        m_animated_component = nullptr;
    SEGS::Model *        m_model              = nullptr;

    uint32_t  m_flags;
    glm::vec3 m_bone_scale;
    glm::vec3 m_bone_delta;
    // material specification
    glm::vec4      m_color_1;
    glm::vec4      m_color_2;
    SEGS::HTexture m_texture_1;
    SEGS::HTexture m_texture_2;
};

// Stupid global to allow converters to know where the unpacked files are
QString basepath;

CharViewerApp::CharViewerApp(Context *ctx) : Application("CoX Costume Viewer", ctx) {}

CharViewerApp::~CharViewerApp() {}

void CharViewerApp::Setup()
{
    engineParameters_[EP_FULL_SCREEN]  = false;
    engineParameters_[EP_WINDOW_TITLE] = m_appName;
    // Force the data directory names to use mapviewer_data/ root
    engineParameters_[EP_RESOURCE_PATHS] = "mapviewer_data/Data;mapviewer_data/CoreData";
}
void cohQuatToMat3(glm::quat a1, glm::mat3 &a2)
{
    float xw = 2 * a1.x * a1.w;
    float yw = 2 * a1.y * a1.w;
    float zw = 2 * a1.z * a1.w;
    float xx = 2 * a1.x * a1.x;
    float yx = 2 * a1.y * a1.x;
    float zx = 2 * a1.z * a1.x;
    float yy = 2 * a1.y * a1.y;
    float zy = 2 * a1.z * a1.y;
    float zz = 2 * a1.z * a1.z;
    a2[0].x  = 1.0f - (zz + yy);
    a2[0].y  = yx - zw;
    a2[0].z  = zx + yw;
    a2[1].x  = yx + zw;
    a2[1].y  = 1.0f - (zz + xx);
    a2[1].z  = zy - xw;
    a2[2].x  = zx - yw;
    a2[2].y  = zy + xw;
    a2[2].z  = 1.0f - (yy + xx);
}
void resetLF3DSkeleton(SEGS::HAnimationTrack src, Skeleton &tgt)
{
    int idx = 0;
    for (Bone &bn : tgt.GetModifiableBones())
    {
        BoneAnimTrack *track = getTrackForBone(src, idx);
        if (track)
        {
            glm::vec3 pos       = track->pos_keys.front();
            glm::quat quat      = track->rot_keys.front();
            bn.initialPosition_ = {pos.x, pos.y, pos.z};
            bn.initialRotation_ = {quat.w, quat.x, quat.y, quat.z};
        }
        idx++;
    }
    tgt.Reset();
}
void cohAnimTrackToLF3D(Urho3D::AnimationTrack &res, BoneAnimTrack &bt)
{
    int                            tick_count = std::max(bt.position_ticks, bt.rotation_ticks);
    std::vector<AnimationKeyFrame> converted_frames;
    converted_frames.resize(tick_count);
    for (int kf_idx = 0; kf_idx < tick_count; ++kf_idx)
    {
        AnimationKeyFrame &kf(converted_frames[kf_idx]);
        kf.time_ = kf_idx / 15.0f;
        if (kf_idx < bt.pos_keys.size())
        {
            kf.position_ = fromGLM(bt.pos_keys[kf_idx]); //
        }
        else if (!bt.pos_keys.empty())
            kf.position_ = fromGLM(bt.pos_keys.back());

        if (kf_idx < bt.rot_keys.size())
        {
            glm::mat3 tgt;
            cohQuatToMat3(bt.rot_keys[kf_idx], tgt);
            glm::quat conv = glm::quat_cast(tgt);
            kf.rotation_   = fromGLM(conv); // bt.rot_keys[kf_idx]
        }
        else if (!bt.rot_keys.empty())
        {
            glm::mat3 tgt;
            cohQuatToMat3(bt.rot_keys.back(), tgt);
            glm::quat conv = glm::quat_cast(tgt);
            kf.rotation_   = fromGLM(conv); // bt.rot_keys[kf_idx]
        }
    }
    if (bt.pos_keys.size() > 1)
        res.channelMask_ |= CHANNEL_POSITION;
    if (bt.rot_keys.size() > 1)
        res.channelMask_ |= CHANNEL_ROTATION;
    res.name_ = SEGS::boneName(bt.tgtBoneOrTexId);
    res.SetAllKeyFrames(converted_frames.data(), converted_frames.size());
}
void CharViewerApp::CreateBaseScene()
{
    // Create a basic plane, a light and a camera
    ResourceCache *cache = GetContext()->m_ResourceCache.get();
    cache->AddResourceDir("./converted");
    m_scene = new Scene(GetContext());
    m_scene->CreateComponent<DebugRenderer>();
    Octree *oct = m_scene->CreateComponent<Octree>();
    oct->SetSize(BoundingBox(-7000, 7000), 8);

    Node *zoneNode = m_scene->CreateChild("Zone");
    Zone *zone     = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    //    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    //    zone->SetFogStart(100.0f);
    //    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-32767, 32768));

    Node *planeNode = m_scene->CreateChild("Plane");
    planeNode->SetPosition({0, -300, 0});
    planeNode->SetScale(Vector3(7000.0f, 1.0f, 7000.0f));
    StaticModel *planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/Checkboard.xml"));

    // A directional light
    Node *lightNode = m_scene->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetSpecularIntensity(0.5f);

    m_camera_node = m_scene->CreateChild("Camera");
    Camera *cam   = m_camera_node->CreateComponent<Camera>();
    m_camera_node->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
    cam->setFarClipDistance(1500);
}

void CharViewerApp::SetupViewport()
{
    Renderer *          renderer = GetContext()->m_Renderer.get();
    SharedPtr<Viewport> viewport(new Viewport(GetContext(), m_scene, m_camera_node->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void CharViewerApp::CreateConsoleAndDebugHud()
{
    // Create console
    engine_->CreateConsole();

    // Create debug HUD.
    engine_->CreateDebugHud();
}

void CharViewerApp::prepareSideWindow()
{
    Graphics *graphics = GetContext()->m_Graphics.get();
    m_sidewindow       = new SideWindow(nullptr);
    m_sidewindow->move(0, graphics->GetWindowPosition().y_);
    m_sidewindow->resize(graphics->GetWindowPosition().x_ - 20, graphics->GetHeight());
    m_sidewindow->show();
    connect(this, &CharViewerApp::modelSelected, m_sidewindow, &SideWindow::onModelSelected);
    connect(m_sidewindow, &SideWindow::costumeSelected, this, &CharViewerApp::onCostumeSelected);
    m_sidewindow->setMapViewer(this);
}

void CharViewerApp::prepareCursor()
{
    ResourceCache *cache = GetContext()->m_ResourceCache.get();
    UI *           ui    = GetContext()->m_UISystem.get();
    Input *        input = GetContext()->m_InputSystem.get();
    XMLFile *      style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor
    // will control the camera, and when visible, it will interact with the scene
    SharedPtr<Cursor> cursor(new Cursor(GetContext()));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    // Set starting position of the cursor at the rendering window center
    input->SetMouseMode(ui->GetCursor()->IsVisible() ? MM_FREE : MM_RELATIVE); //,MM_RELATIVE
}

void CharViewerApp::Start()
{
    QSettings our_settings(QSettings::IniFormat, QSettings::UserScope, "SEGS", "MapViewer");
    basepath = our_settings.value("ExtractedDir", QString()).toString();
    if (!basepath.endsWith('/'))
        basepath += '/';
    CreateBaseScene();
    SetupViewport();
    CreateConsoleAndDebugHud();
    prepareCursor();

    // Subscribe to key events
    g_inputSignals.keyDown.Connect(this, &CharViewerApp::HandleKeyDown);
    g_inputSignals.keyUp.Connect(this, &CharViewerApp::HandleKeyUp);
    // Subscribe to update ticks
    g_coreSignals.update.Connect(this, &CharViewerApp::HandleUpdate);
    g_coreSignals.postRenderUpdate.Connect(this, &CharViewerApp::HandlePostRenderUpdate);

    Input *input = GetContext()->m_InputSystem.get();
    input->SetMouseMode(MM_FREE); //,MM_RELATIVE
    if (!getGameData().read_game_data(basepath))
    {
        QMessageBox::critical(nullptr, "Missing files", "At least some of the required bin/* files are missing");
        exit(1);
    }
    if (!getRuntimeData().prepare(basepath))
    {
        QMessageBox::critical(nullptr, "Missing files", "At least some of the required bin/* files are missing");
        exit(1);
    }

    SEGS::preloadTextureNames(basepath);
    prepareSideWindow();
}

void CharViewerApp::loadSelectedSceneGraph(const QString &path)
{
    m_selected_drawable = nullptr;
    for (auto v : m_converted_nodes)
    {
        m_scene->RemoveChild(v.second);
    }
    m_converted_nodes.clear();
    m_coh_scene.reset(SEGS::loadWholeMap(path));
}
struct SequencerInfo
{
    QString name;
};

struct EntitySequencerInstance
{
    SEGS::SequencerInstance *m_sequencer_instance;
    SequencerInfo            m_seq_type_info;
};
struct RenderableCharacter
{
    Parse_Costume *          m_currently_applied_costume;
    EntitySequencerInstance *m_entity_sequencer;
};
// FIXME: original code does not set alpha in custom colors, even though some callers pass RGBA values to it
void setCustomNodeColors(Node *node, const SEGS::ModelPartColor &colors)
{
    // TODO:    node->m_flags |= 0x200000;
    node->SetVar("Color1", Urho3D::Color(colors.color1.r, colors.color1.g, colors.color1.b));
    node->SetVar("Color2", Urho3D::Color(colors.color2.r, colors.color2.g, colors.color2.b));
}
void setNodeTextures(Node *node, SEGS::HTexture t1, SEGS::HTexture t2)
{
    if (!node)
        return;
    if (!(t1 || t2))
        return;
    SEGS::HTexture white_tex = tryLoadTexture(node->GetContext(), "white.tga");

    if (!t1)
        t1 = white_tex;
    if (!t2)
        t2 = white_tex;
    node->SetVar("Texture1", t1.toInt());
    node->SetVar("Texture2", t2.toInt());
    // TODO: node->m_flags |= 0x40000;
}
void animBuildSkeleton(std::vector<SEGS::BoneLink> &bone_link_arr, int start_bone_id, Node *parent,
                       SEGS::HSequencerInstance seq, std::vector<Bone> &converted)
{
    SEGS::BoneLink *bone_link = nullptr;

    for (int idx = start_bone_id; idx != -1; idx = bone_link->next_bone_idx)
    {
        assert(SEGS::legitBone(idx));
        if (idx >= converted.size())
            converted.resize(idx + 1);
        Bone &b(converted[idx]);

        bone_link = &bone_link_arr[idx];
        assert(bone_link && idx == bone_link->id);
        Urho3D::Node *skel_node = parent->GetChild(SEGS::boneName(idx));
        if (!skel_node)
            skel_node = parent->CreateChild(SEGS::boneName(idx));
        b.node_        = skel_node;
        b.parentIndex_ = start_bone_id;
        b.name_        = SEGS::boneName(idx);

        skel_node->SetVar("BoneSlot", bone_link->id);
        // TODO: if ( seq->m_seq_type_info.m_light_as_door_outside )
        //            char_node->m_flags |= 0x1000000;
        //        char_node->m_flags |= 0x400000;
        skel_node->SetVar("BoneScale", fromGLM(seq->m_bone_scales[bone_link->id]));
        setCustomNodeColors(skel_node, seq->m_part_colors[idx]);
        setNodeTextures(skel_node, seq->m_bone_textures_1[idx], seq->m_bone_textures_2[idx]);
        // char_node->rgbs = (uint32_t *)seq->m_bone_vertices_arr[idx];
        seq->m_interpolation_state[bone_link->id] |= 1u;
        seq->m_interpolation_state[bone_link->id] |= 2u;
        if (bone_link->child_id != -1)
            animBuildSkeleton(bone_link_arr, bone_link->child_id, skel_node, seq, converted);
    }
}
void getBaseRotation(BoneAnimTrack *a1, Matrix4 &a2)
{
    glm::mat3 tgt;
    cohQuatToMat3(a1->rot_keys[0], tgt);
    a2.m00_ = tgt[0][0];
    a2.m01_ = tgt[0][1];
    a2.m02_ = tgt[0][2];
    a2.m10_ = tgt[1][0];
    a2.m11_ = tgt[1][1];
    a2.m12_ = tgt[1][2];
    a2.m20_ = tgt[2][0];
    a2.m21_ = tgt[2][1];
    a2.m22_ = tgt[2][2];
}
static void animAttachBoneTracks(SEGS::HAnimationTrack skeleton_track, SEGS::HAnimationTrack anim,
                                 LF3CharacterNode *node)
{

    assert(skeleton_track && !skeleton_track->m_bone_tracks.empty());
    if (!node)
        return;

    resetLF3DSkeleton(skeleton_track, *node->m_skeleton);
    Context *  ctx            = node->m_node->GetContext();
    Animation *converted_anim = ctx->m_ResourceCache->GetResource<Animation>(skeleton_track->m_name);
    if (!converted_anim)
    {
        converted_anim = new Urho3D::Animation(ctx);
        converted_anim->SetName(skeleton_track->m_name);
        ctx->m_ResourceCache->AddManualResource(converted_anim);
        int max_length = 0;
        for (const SEGS::BoneLink &bl : skeleton_track->m_skeleton_hierarchy)
        {
            // TODO: this will not rebase animation skeleton_track to start of `anim` pose
            BoneAnimTrack *bat = getTrackForBone(skeleton_track, bl.id);
            if (bat)
                ; // TODO: node->m_flags &= ~0x800000u;
            else
            {
                bat = getTrackForBone(skeleton_track->m_backup_anim_track, bl.id);
                // TODO: mark node->m_flags |= 0x800000;
            }
            if (bat)
            {
                max_length         = std::max<int>(max_length, bat->pos_keys.size());
                max_length         = std::max<int>(max_length, bat->rot_keys.size());
                AnimationTrack *at = converted_anim->CreateTrack(SEGS::boneName(bl.id));
                cohAnimTrackToLF3D(*at, *bat);
            }
        }
        converted_anim->SetLength(max_length / 15.0f);
        node->m_anim_controller->Play(skeleton_track->m_name, 0, true);
    }

    node->m_anim_state = node->m_anim_controller->GetAnimationState(skeleton_track->m_name);
    node->m_anim_state->SetLooped(true);
}
SEGS::Model *animGetGeometryForThisBone(int id, SEGS::HSequencerInstance hseq)
{
    static const char *s_lod_extensions[] = {
        "", "_LOD1", "_LOD2", "_LOD3", "_LOD4",
    };
    SEGS::SequencerInstance &seq_inst(hseq.get());
    SEGS::RuntimeData &      rd(getRuntimeData());
    SEGS::PrefabStore *      pf(rd.m_prefab_mapping);
    char                     buf[100];
    char                     model_name[100];
    QString                  filename;
    SEGS::Model *            result = nullptr;

    assert(SEGS::legitBone(id));
    if (0 == hseq->m_bone_geometry_names[id].compare("NONE", Qt::CaseInsensitive))
        return nullptr;

    if (SEGS::legitBone(id))
    {
        if (!hseq->m_bone_geometry_names[id].isEmpty())
        {
            strcpy(buf, hseq->m_bone_geometry_names[id].data());
            char *next_dot = strtok(buf, ".");
            if (hseq->m_lod_level)
                filename = hseq->m_seq_type_info.lod_Name(hseq->m_lod_level);
            else
                filename = QString("player_library/") + next_dot + ".geo";
            char *next_tok = strtok(nullptr, "");
            sprintf(model_name, "%s%s", next_tok, s_lod_extensions[hseq->m_lod_level]);
            result = pf->modelFind(filename, model_name);
            if (!result && hseq->m_lod_level == 1)
            {
                filename = QString("player_library/") + next_dot + ".geo";
                sprintf(model_name, "%s%s", next_tok, "");
                result = pf->modelFind(model_name, filename);
            }
            if (!result && !hseq->m_lod_level)
            {
                qCritical() << " ## ERROR! ## ";
                qCritical() << " %s asked for bad custom geometry: ", qPrintable(hseq->m_seq_type_info.m_name);
                qCritical() << " %s at %s", model_name, qPrintable(filename);
            }
        }
    }
    if (!result)
    {
        if (!hseq->m_seq_type_info.lod_Name(hseq->m_lod_level).isEmpty())
        {
            sprintf(buf, "GEO_%s%s", SEGS::boneName(id), s_lod_extensions[hseq->m_lod_level]);
            result = pf->modelFind(hseq->m_seq_type_info.lod_Name(hseq->m_lod_level), buf);
        }
    }
    return result;
}

void animAttachGeometryToSkeleton(Node *node, SEGS::HSequencerInstance seq)
{
    if (node->GetVars().find("BoneSlot") != node->GetVars().end())
    {
        int bone_slot = node->GetVar("BoneSlot").GetInt();
        assert(SEGS::legitBone(bone_slot));
        SEGS::Model *model = animGetGeometryForThisBone(bone_slot, seq);
        if (model)
        {
            convertedModelToLutefisk(node, model, 0, 1000.0f);
            //            node->m_animated_component->SetModel(node->m_model);
            //            gfxTreeInitGfxNodeWithObjectsTricks(node);
            //            node->m_model->model_VisSphereRadius = a2->m_seq_type_info.m_visibility_sphere_radius;
        }
    }
    for (auto child : node->GetChildren())
    {
        animAttachGeometryToSkeleton(child, seq);
    }
}
void setColorsAndTextures(SEGS::HSequencerInstance a1, Node *node)
{
    if (node->GetVars().find("BoneSlot") != node->GetVars().end())
    {
        int bone_slot = node->GetVar("BoneSlot").GetInt();
        if (node->HasComponent<StaticModel>())
        {
            // TODO: modify the model materials here
            if (SEGS::legitBone(bone_slot))
            {
                setCustomNodeColors(node, a1->m_bone_colors[bone_slot]);
                setNodeTextures(node, a1->m_bone_textures_1[bone_slot], a1->m_bone_textures_2[bone_slot]);
            }
        }
    }
    for (auto v : node->GetChildren())
        setColorsAndTextures(a1, v);
}

void animSetHeader(SEGS::HSequencerInstance seq)
{
    using namespace SEGS;

    SeqMove *anim = &seq->m_anim;
    // TODO: verify that SceneNode has been instantiated into a SceneTreeNode ?
    LF3CharacterNode *char_node = dynamic_cast<LF3CharacterNode *>(seq->m_node);
    assert(char_node);
    SeqTypeDefData *seq_typedef   = getSeqTypedefByName(seq->m_template, seq->m_seq_type_info.m_seq_type);
    seq->m_animation.m_base_track = getOrLoadAnimationTrack(seq_typedef->pBaseSkeleton);
    HAnimationTrack atrack        = seq->m_animation.m_base_track;
    AnimTrack &     track_ref(atrack.get());
    if (!seq->m_animation.m_base_track)
        qCritical() << QString("Animation %1 doesn't exist. Sequencer %2 wants it for a base skeleton")
                           .arg(QString(seq_typedef->pBaseSkeleton), QString(seq->m_seq_type_info.m_seq_type));
    assert(seq->m_animation.m_base_track && anim->move);
    seq->m_curr_interpolated_frame = 0;
    seq->m_interpolation_state.fill(0);
    if (char_node->m_anim_controller && char_node->m_anim_state)
    {
        char_node->m_anim_controller->Stop(char_node->m_anim_state->GetAnimation()->GetName());
        char_node->m_anim_controller->Update(0);
        char_node->m_anim_state = nullptr;
    }

    qWarning() << "Not deleting animation";
    //    if (gfxTreeNodeIsValid(seq->m_node, seq->gfx_root_unique_id))
    //        gfxTreeDeleteAnimation(seq->m_node, seq);
    char_node->m_rendering_data = &seq->m_rendering_data;
    std::vector<Bone> converted;
    animBuildSkeleton(seq->m_animation.m_base_track->m_skeleton_hierarchy,
                      seq->m_animation.m_base_track->m_skeleton_hierarchy[0].id, char_node->m_node, seq, converted);
    delete char_node->m_skeleton;
    char_node->m_skeleton                       = new Skeleton;
    char_node->m_skeleton->GetModifiableBones() = converted;
    char_node->m_skeleton->SetRootBoneIndex(atrack->m_skeleton_hierarchy[0].id);
    animAttachBoneTracks(seq->m_anim.type->m_anm_track, SEGS::HAnimationTrack(seq->m_animation.m_base_track),
                         char_node);
    animAttachGeometryToSkeleton(char_node->m_node, seq);
    setColorsAndTextures(seq, char_node->m_node);
    qWarning() << "Not linking inactive bones";
    //    if (!gfxTreeRelinkSuspendedNodes(seq->m_node, seq->seq_instance_handle))
    //        qCritical("     (missing model bone: %s) \n", qPrintable(anim->move->m_Type[0].m_Anim[0].name));
    qWarning() << "Not checking for async loaded objects";
    // seq->loadingObjects = animCheckForLoadingObjects(seq->m_node, seq->seq_instance_handle);
    // animCalcObjAndBoneUse((LF3CharacterNode *)seq->m_node, seq);
}
SEGS::HSequencerInstance seqLoadInst(const QString &name, int rand_seed, SEGS::SceneTreeNode *node)
{
    assert(node);
    SEGS::HSequencerInstance seq = SEGS::SequencerInstanceStorage::instance().create(node);
    assert(seq);
    SEGS::seqResetSeqType(seq, qPrintable(name), rand_seed);
    animSetHeader(seq);
    return seq;
}
void setSeqeuncerInstance(DisplayedCharacter &costume, SEGS::HSequencerInstance seq_instance)
{
    costume.m_sequencer = seq_instance;
    // todo: create a collision mesh here.
}
void changeCharacterSequencer(DisplayedCharacter &displayed_char, const QString &bodyname)
{
    using namespace SEGS;
    const int rand_seed = 42;
    assert(!bodyname.isEmpty());
    HSequencerInstance sequencer = displayed_char.m_sequencer;
    if (sequencer) // already initialized.
    {
        if (0 == bodyname.compare(displayed_char.m_data.m_name, Qt::CaseInsensitive))
            return;
        // release old sequencer.
        sequencer.destroy();
    }
    HSequencerInstance seq_inst = seqLoadInst(bodyname, rand_seed, displayed_char.m_node);
    setSeqeuncerInstance(displayed_char, seq_inst);
    return;
}
static void scaleHero(SEGS::HSequencerInstance seq, float scale)
{
    if (!seq)
        return;

    float     factor    = scale / 100.0f + 1.0f;
    glm::vec3 scale_vec = {factor, factor, factor};
    changeSequencerScale(seq, scale_vec);
}

void changeBoneColorization(SEGS::HSequencerInstance hseq, int bone_idx, const SEGS::ModelPartColor &mpc)
{
    assert(hseq);
    assert(SEGS::legitBone(bone_idx));
    hseq->m_bone_has_color[bone_idx]       = 1;
    SEGS::ModelPartColor prev_val          = hseq->m_bone_colors[bone_idx];
    hseq->m_bone_colors[bone_idx]          = mpc;
    hseq->m_bone_colors[bone_idx].color1.a = 1;
    hseq->m_bone_colors[bone_idx].color2.a = 1;
    hseq->m_updated_appearance |= !(prev_val == mpc);
}
static void setupModelPartColors(RGBA skincolor, RGBA col1, RGBA col2, SEGS::ModelPartColor &tgt1, const QString &name)
{
    if (name.contains("SKIN", Qt::CaseInsensitive))
    {
        tgt1.color1 = skincolor.toFloats();
        tgt1.color2 = col1.toFloats();
    }
    else
    {
        tgt1.color1 = col1.toFloats();
        tgt1.color2 = col2.toFloats();
    }
}
void changeBodyPartColorization(SEGS::HSequencerInstance hseq, BodyPart_Data *bp, RGBA skin_color, RGBA color1,
                                RGBA color2, const QString &texname)
{
    if (bp->boneIndices[0] == -1)
        return;
    SEGS::ModelPartColor mpc;
    setupModelPartColors(skin_color, color1, color2, mpc, texname);
    changeBoneColorization(hseq, bp->boneIndices[0], mpc);
    if (bp->m_BoneCount == 2)
        changeBoneColorization(hseq, bp->boneIndices[1], mpc);
}
void changeBoneGeometry(SEGS::HSequencerInstance hseq, int bone, const QByteArray &geom_name)
{
    SEGS::RuntimeData &rd(getRuntimeData());
    auto               prefab_store = rd.m_prefab_mapping;
    bool               result       = true;
    QByteArray         bonename     = geom_name;

    assert(hseq);
    assert(SEGS::legitBone(bone));

    if (!geom_name.isEmpty() && 0 != geom_name.compare("NONE"))
    {
        auto    refparts = geom_name.split('.');
        QString filename = QString("player_library/") + refparts[0] + ".geo";
        if (!prefab_store->modelFind(filename, refparts[1]))
        {
            if (refparts[1].toLower().contains("larm"))
                bonename = "NONE";
            else
                result = false;
        }
    }
    if (bonename.isEmpty())
        return;
    // if already proper name is assigned, no need to set `m_updated_appearance`
    if (0 == hseq->m_bone_geometry_names[bone].compare(geom_name, Qt::CaseInsensitive))
        return;

    hseq->m_updated_appearance        = 1;
    hseq->m_bone_geometry_names[bone] = bonename;
    if (!result)
    {
        hseq->m_bone_geometry_names[bone] += "BAD!";
    }
}

void changeBodyPartGeometry(SEGS::HSequencerInstance hseq, const Parse_Costume *costume, BodyPart_Data *bodypart,
                            const QByteArray &geom_name)
{
    if ((geom_name.isEmpty() || !bodypart || bodypart->boneIndices[0] == -1))
        return;

    if (0 == geom_name.compare("NONE", Qt::CaseInsensitive))
    {
        changeBoneGeometry(hseq, bodypart->boneIndices[0], "NONE");
        if (bodypart->m_BoneCount == 2)
            changeBoneGeometry(hseq, bodypart->boneIndices[1], "NONE");
        return;
    }

    QByteArray buf =
        QString("%1_%2.GEO_%3").arg(entTypeFileName(costume), bodypart->m_BaseName, bodypart->m_GeoName).toLatin1();
    QByteArray buf3 = buf;

    if (bodypart->m_BoneCount == 2)
        buf3 += "R";

    changeBoneGeometry(hseq, bodypart->boneIndices[0], buf3 + "_" + geom_name);
    if (bodypart->m_BoneCount == 2)
    {
        changeBoneGeometry(hseq, bodypart->boneIndices[1], buf + "L_" + geom_name);
    }
}
void changeBoneTexture(Urho3D::Context *ctx, SEGS::HSequencerInstance hseq, int bone, const QByteArray &base_tex,
                       const QByteArray &detail_tex)
{

    assert(hseq);
    assert(SEGS::legitBone(bone));
    SEGS::HTexture white_tex = tryLoadTexture(ctx, "white.tga");
    SEGS::HTexture bone_tex_1;

    if (!base_tex.isEmpty())
    {
        bone_tex_1 = white_tex;
        if (base_tex.toLower() != "none")
        {
            bone_tex_1 = tryLoadTexture(ctx, base_tex);
            if (!bone_tex_1 || (bone_tex_1 == white_tex && base_tex.toLower() != "white"))
            {
                qCritical() << "changeBoneTexture: failed to locate textur" << base_tex << "for bone"
                            << SEGS::boneName(bone) << "in seq" << hseq->m_seq_type_info.m_name;
            }
        }
    }
    hseq->m_bone_textures_1[bone] = bone_tex_1;
    SEGS::HTexture bone_tex_2;
    if (!detail_tex.isEmpty())
    {
        bone_tex_2 = white_tex;
        if (detail_tex.toLower() != "none")
        {
            bone_tex_2 = tryLoadTexture(ctx, detail_tex);
            if (!bone_tex_2 || (bone_tex_2 == white_tex && detail_tex.toLower() != "white"))
            {
                qCritical() << "changeBoneTexture: failed to locate textur" << detail_tex << "for bone"
                            << SEGS::boneName(bone) << "in seq" << hseq->m_seq_type_info.m_name;
            }
        }
    }
    hseq->m_bone_textures_2[bone] = bone_tex_2;
    hseq->m_updated_appearance    = 1;
}
void changePartTexture(Urho3D::Context *ctx, SEGS::HSequencerInstance hseq, Parse_Costume *costume, BodyPart_Data *bp,
                       const QByteArray &texname1, const QByteArray &texname2, bool ends_with_x, bool ends_with_a)
{
    bool       tex2_is_none;
    QByteArray second;
    QByteArray buf;
    int        result = 0;

    if (texname1.isEmpty() || !bp || bp->boneIndices[0] == -1)
        return;
    tex2_is_none = texname1.compare("NONE", Qt::CaseInsensitive) == 0;
    if (tex2_is_none)
    {
        buf    = "none";
        result = 1;
    }
    else
    {
        buf = QByteArray(bp->m_TexName) + "_" + texname1;
    }
    tex2_is_none = texname2.isEmpty() || texname2.compare("NONE", Qt::CaseInsensitive) == 0;
    if (tex2_is_none || ends_with_x)
    {
        second = "none";
    }
    else if (ends_with_a)
    {
        if (buf.toLower().endsWith("01a"))
        {
            second                    = buf;
            second[second.size() - 1] = 'B';
        }
        else
        {
            second = QByteArray(bp->m_TexName) + "_" + texname2;
        }
    }
    else if (!tex2_is_none)
    {
        second = QByteArray(bp->m_TexName) + "_" + texname2;
    }
    else
    {
        second = "none";
    }
    QByteArray name2 = bodytype_prefix_fixup(costume, buf);
    if (tryLoadTexture(ctx, name2 + ".tga"))
    {
        buf = name2;
    }
    QByteArray name3 = bodytype_prefix_fixup(costume, second);
    if (tryLoadTexture(ctx, name3 + ".tga"))
    {
        second = name3;
    }
    for (int i = 0; i < bp->m_BoneCount; ++i)
        changeBoneTexture(ctx, hseq, bp->boneIndices[i], buf, second);
}
void CharViewerApp::onCostumeSelected(const Parse_Costume *n)
{
    qDebug() << "TODO: Convert costume to full set of nodes!";
    QString ent_type = entTypeFileName(n);
    qDebug() << "Selected ent type" << ent_type;
    m_character.m_costume_tpl = n;
    if (!m_currently_shown_node)
    {
        m_currently_shown_node       = m_scene->CreateChild("Avatar");
        LF3CharacterNode *node_wrap  = new LF3CharacterNode;
        node_wrap->m_node            = m_currently_shown_node;
        node_wrap->m_anim_controller = m_currently_shown_node->GetComponent<AnimationController>();
        if (!node_wrap->m_anim_controller)
            node_wrap->m_anim_controller = m_currently_shown_node->CreateComponent<AnimationController>();
        m_character.m_node = node_wrap;
        m_camera_node->LookAt(m_currently_shown_node->GetWorldPosition());
    }
    changeCharacterSequencer(m_character, ent_type);
    // Vector3 nodecenter = m_currently_shown_node->GetWorldPosition();//
    // fromGLM(m_currently_shown_node->m_bbox.center()); Vector3 dir = (m_camera_node->GetWorldPosition() -
    // nodecenter).Normalized(); Vector3 newpos = nodecenter + dir * n->m_bbox.size().length()*1.5f;
    // m_camera_node->SetPosition(newpos);
    Quaternion camrot = m_camera_node->GetRotation();
    pitch_            = camrot.PitchAngle();
    yaw_              = camrot.YawAngle();
    //    m_current_selected_node = n;
    if (0.0f != m_character.m_costume_tpl->m_Scale)
        scaleHero(m_character.m_sequencer, m_character.m_costume_tpl->m_Scale);
    if (0.0f != m_character.m_costume_tpl->m_BoneScale)
        changeBoneScale(m_character.m_sequencer, m_character.m_costume_tpl->m_BoneScale);
    BodyPartsStorage &bodyparts(getGameData().m_body_parts);
    BodyPart_Data *   bodyPart;
    for (uint32_t part_idx = 0; part_idx < m_character.m_costume_tpl->m_NumParts; ++part_idx)
    {
        if (bodyparts.m_parts[part_idx].m_BoneCount <= 0)
            continue;
        const Parse_CostumePart &pcp(m_character.m_costume_tpl->m_CostumeParts[part_idx]);
        if (!pcp.m_Name.isEmpty())
            bodyPart = bodyparts.getBodyPartFromName(pcp.m_Name);
        else
            bodyPart = &bodyparts.m_parts[part_idx];
        changeBodyPartGeometry(m_character.m_sequencer, n, bodyPart, pcp.m_CP_Geometry);
        bool bSingleTexture = pcp.m_Texture1.toLower().endsWith('x');
        bool bDualPass      = pcp.m_Texture1.toLower().endsWith('a');
        qDebug() << "Not changing textures";
        // doChangeTex(m_character.m_sequencer, bodyPart, pcp.m_Texture1, pcp.m_Texture2, bSingleTexture, bDualPass);
        changeBodyPartColorization(m_character.m_sequencer, bodyPart, m_character.m_costume_tpl->m_SkinColor,
                                   pcp.m_Color1, pcp.m_Color2, pcp.m_Texture1);
    }
}

#define MAX_GRAPH_DEPTH 80

int created_node_count = 0;

void CharViewerApp::HandleKeyUp(int key, int scancode, unsigned buttons, int qualifiers)
{
    // Close console (if open) or exit when ESC is pressed
    if (key == KEY_ESCAPE)
    {
        engine_->Exit();
    }
    if (key >= '1' && key <= '8')
    {
        int layernum = key - '1';
        if (layernum >= m_coh_scene->refs.size())
            return;
        auto  ref_to_toggle = m_coh_scene->refs[layernum];
        Node *v             = m_converted_nodes[ref_to_toggle];
        if (v)
            v->SetDeepEnabled(!v->IsEnabled());
    }
}

void CharViewerApp::HandleKeyDown(int key, int /*scancode*/, unsigned /*buttons*/, int /*qualifiers*/, bool /*repeat*/)
{
    // Toggle console with F1
    if (key == KEY_F1)
        GetContext()->GetSubsystemT<Console>()->Toggle();

    // Toggle debug HUD with F2
    else if (key == KEY_F2)
        GetContext()->GetSubsystemT<DebugHud>()->ToggleAll();
    if (key == '9')
    {
        Graphics *graphics = GetContext()->m_Graphics.get();
        Image     screenshot(GetContext());
        graphics->TakeScreenShot(screenshot);
        // Here we save in the Data folder with date and time appended
        screenshot.SavePNG("Data/Screenshot_" +
                           Time::GetTimeStamp().replace(':', '_').replace('.', '_').replace(' ', '_') + ".png");
    }
    else if (key == 'g')
    {
    }
}

bool CharViewerApp::Raycast(float maxDistance)
{
    Vector3   hitPos;
    Drawable *hitDrawable = nullptr;

    UI *       ui  = GetContext()->m_UISystem.get();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (!ui->GetCursor()->IsVisible() || ui->GetElementAt(pos, true))
        return false;

    Graphics *graphics = GetContext()->m_Graphics.get();
    Camera *  camera   = m_camera_node->GetComponent<Camera>();
    Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    std::vector<RayQueryResult> results;
    RayOctreeQuery              query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    m_scene->GetComponent<Octree>()->RaycastSingle(query);
    if (!results.empty())
    {
        RayQueryResult &result = results[0];
        hitPos                 = result.position_;
        hitDrawable            = result.drawable_;
        Variant stored         = result.node_->GetVar("CoHModel");
        Variant stored_node    = result.node_->GetVar("CoHNode");
        if (stored != Variant::EMPTY)
        {
            m_selected_drawable = hitDrawable;
            emit modelSelected((SEGS::SceneNode *)stored_node.GetVoidPtr(), (SEGS::Model *)stored.GetVoidPtr(),
                               hitDrawable);
        }
        return true;
    }
    m_selected_drawable = nullptr;
    emit modelSelected(nullptr, nullptr, nullptr);
    return false;
}

void CharViewerApp::HandleUpdate(float timeStep)
{
    Input *input = GetContext()->m_InputSystem.get();
    qApp->processEvents();
    UI *ui = GetContext()->m_UISystem.get();
    ui->GetCursor()->SetVisible(!input->GetMouseButtonDown(MouseButton::MOUSEB_RIGHT));
    input->SetMouseMode(ui->GetCursor()->IsVisible() ? MM_FREE : MM_RELATIVE); //,MM_RELATIVE
    LF3CharacterNode *c_node = (LF3CharacterNode *)m_character.m_node;

    if (c_node && c_node->m_anim_state)
        c_node->m_anim_state->AddTime(timeStep);
    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;
    if (!ui->GetCursor()->IsVisible())
    {
        // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
        m_camera_node->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
    }
    bool shifted = input->GetKeyDown(KEY_LEFT_SHIFT);
    if (shifted)
        timeStep *= 10;
    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    bool cameraLocationUpdated = false;
    if (input->GetKeyDown(KEY_W))
    {
        m_camera_node->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_S))
    {
        m_camera_node->Translate(Vector3::BACK * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_A))
    {
        m_camera_node->Translate(Vector3::LEFT * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_D))
    {
        m_camera_node->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_PAGE_UP) || input->GetKeyDown(KEY_Q))
    {
        m_camera_node->Translate(Vector3::UP * MOVE_SPEED * timeStep * 5);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_PAGE_DOWN) || input->GetKeyDown(KEY_E))
    {
        m_camera_node->Translate(Vector3::DOWN * MOVE_SPEED * timeStep * 5);
        cameraLocationUpdated = true;
    }
    if (cameraLocationUpdated)
    {
        Vector3 pos = m_camera_node->GetPosition();
    }
    if (ui->GetCursor()->IsVisible() && input->GetMouseButtonPress(MouseButton::MOUSEB_LEFT))
        Raycast(8500);
}

void CharViewerApp::HandlePostRenderUpdate(float /*ts*/)
{
    // If draw debug mode is enabled, draw viewport debug geometry. Disable depth test so that we can see the effect of
    // occlusion
    LF3CharacterNode *c_node = (LF3CharacterNode *)m_character.m_node;
    if (c_node && c_node->m_skeleton)
    {
        m_scene->GetComponent<DebugRenderer>()->AddSkeleton(*c_node->m_skeleton, Color::GREEN);
        if (m_character.m_sequencer && m_character.m_sequencer->m_updated_appearance)
        {
            animSetHeader(m_character.m_sequencer);
            m_character.m_sequencer->m_updated_appearance = false;
        }
    }
    if (m_selected_drawable)
    {
        m_selected_drawable->DrawDebugGeometry(m_scene->GetComponent<DebugRenderer>(), true);
    }
    if (!m_current_selected_node)
        return;
    BoundingBox bbox(fromGLM(m_current_selected_node->m_bbox.m_min), fromGLM(m_current_selected_node->m_bbox.m_max));
    m_scene->GetComponent<DebugRenderer>()->AddBoundingBox(bbox, Color::BLUE, false);
}

//! @}
