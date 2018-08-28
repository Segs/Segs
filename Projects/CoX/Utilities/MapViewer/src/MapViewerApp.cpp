/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "MapViewerApp.h"
#include "CoHSceneConverter.h"
#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "CohTextureConverter.h"
#include "SideWindow.h"

#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Core/CoreEvents.h>
#include <Lutefisk3D/Engine/Engine.h>
#include <Lutefisk3D/Input/InputEvents.h>
#include <Lutefisk3D/Input/Input.h>
#include <Lutefisk3D/Engine/EngineDefs.h>
#include <Lutefisk3D/UI/Text3D.h>
#include <Lutefisk3D/UI/Font.h>
#include <Lutefisk3D/UI/UI.h>
#include <Lutefisk3D/UI/Console.h>
#include <Lutefisk3D/UI/DebugHud.h>
#include <Lutefisk3D/UI/BorderImage.h>
#include <Lutefisk3D/Scene/Scene.h>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/Graphics/Octree.h>
#include <Lutefisk3D/Graphics/Camera.h>
#include <Lutefisk3D/Graphics/DebugRenderer.h>
#include <Lutefisk3D/Graphics/Light.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Graphics/Model.h>
#include <Lutefisk3D/Graphics/Material.h>
#include <Lutefisk3D/Graphics/Renderer.h>
#include <Lutefisk3D/Graphics/Zone.h>
#include <Lutefisk3D/Resource/ResourceCache.h>

#include <QSettings>
#include <QtCore/QDebug>
#include <QtWidgets/QApplication>

using namespace Urho3D;

// Stupid global to allow converters to know where the unpacked files are
QString basepath;

MapViewerApp::MapViewerApp(Context * ctx) : Application("CoX Map Viewer",ctx)
{
}

MapViewerApp::~MapViewerApp()
{
}

void MapViewerApp::Setup()
{
    engineParameters_[EP_FULL_SCREEN]  = false;
    engineParameters_[EP_WINDOW_TITLE] = m_appName;
    //Force the data directory names to use mapviewer_data/ root
    engineParameters_[EP_RESOURCE_PATHS] = "mapviewer_data/Data;mapviewer_data/CoreData";
}

void MapViewerApp::CreateBaseScene()
{
    // Create a basic plane, a light and a camera
    ResourceCache* cache = m_context->m_ResourceCache.get();
    m_scene = new Scene(m_context);
    m_scene->CreateComponent<DebugRenderer>();
    Octree *oct=m_scene->CreateComponent<Octree>();
    oct->SetSize(BoundingBox(-32767,32768),8);

    Node* zoneNode = m_scene->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
//    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
//    zone->SetFogStart(100.0f);
//    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-32767, 32768));

    Node* planeNode = m_scene->CreateChild("Plane");
    planeNode->SetPosition({0,-300,0});
    planeNode->SetScale(Vector3(7000.0f, 1.0f, 7000.0f));
    StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/Checkboard.xml"));

    // A directional light
    Node* lightNode = m_scene->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetSpecularIntensity(0.5f);

    m_camera_node = m_scene->CreateChild("Camera");
    Camera *cam = m_camera_node->CreateComponent<Camera>();
    m_camera_node->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
    emit cameraLocationChanged(0.0f, 5.0f, 0.0f);
    cam->SetFarClip(1500);
}

void MapViewerApp::SetupViewport() {
    Renderer *renderer = m_context->m_Renderer.get();
    SharedPtr<Viewport> viewport(
                new Viewport(m_context, m_scene, m_camera_node->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void MapViewerApp::CreateConsoleAndDebugHud()
{
    // Get default style
    ResourceCache* cache = m_context->m_ResourceCache.get();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.8f);
    console->SetNumBufferedRows(20);
    console->SetNumHistoryRows(0);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);
}

void MapViewerApp::prepareSideWindow()
{
    Graphics *graphics = m_context->m_Graphics.get();
    m_sidewindow = new SideWindow(nullptr);
    m_sidewindow->move(0,graphics->GetWindowPosition().y_);
    m_sidewindow->resize(graphics->GetWindowPosition().x_-20,graphics->GetHeight());
    m_sidewindow->show();
    connect(this,&MapViewerApp::cameraLocationChanged,m_sidewindow,&SideWindow::onCameraPositionChanged);
    connect(this,&MapViewerApp::modelSelected,m_sidewindow,&SideWindow::onModelSelected);
    connect(this,&MapViewerApp::scenegraphLoaded,m_sidewindow,&SideWindow::onScenegraphLoaded);
    connect(m_sidewindow,&SideWindow::scenegraphSelected,this,&MapViewerApp::loadSelectedSceneGraph);
    connect(m_sidewindow,&SideWindow::nodeDisplayRequest,this,&MapViewerApp::onDisplayNode);
    connect(m_sidewindow,&SideWindow::refDisplayRequest,this,&MapViewerApp::onDisplayRef);
    connect(m_sidewindow,&SideWindow::nodeSelected,this,&MapViewerApp::onNodeSelected);
    m_sidewindow->setMapViewer(this);
}

void MapViewerApp::prepareCursor()
{
    ResourceCache* cache = m_context->m_ResourceCache.get();
    UI* ui = m_context->m_UISystem.get();
    Input* input = m_context->m_InputSystem.get();
    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor
    // will control the camera, and when visible, it will interact with the scene
    SharedPtr<Cursor> cursor(new Cursor(m_context));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    // Set starting position of the cursor at the rendering window center
    input->SetMouseMode(ui->GetCursor()->IsVisible() ? MM_FREE : MM_RELATIVE);//,MM_RELATIVE

}

void MapViewerApp::Start()
{
    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    basepath = our_settings.value("ExtractedDir",QString()).toString();
    if(!basepath.endsWith('/'))
        basepath+='/';
    CreateBaseScene();
    SetupViewport();
    CreateConsoleAndDebugHud();
    prepareCursor();

    // Subscribe to key events
    g_inputSignals.keyDown.Connect(this,&MapViewerApp::HandleKeyDown);
    g_inputSignals.keyUp.Connect(this,&MapViewerApp::HandleKeyUp);
    // Subscribe to update ticks
    g_coreSignals.update.Connect(this,&MapViewerApp::HandleUpdate);
    g_coreSignals.postRenderUpdate.Connect(this,&MapViewerApp::HandlePostRenderUpdate);

    Input* input = m_context->m_InputSystem.get();
    input->SetMouseMode(MM_FREE);//,MM_RELATIVE

    if(!prepareGeoLookupArray())
        return;
    preloadTextureNames();
    prepareSideWindow();
}

void MapViewerApp::loadSelectedSceneGraph(const QString &path)
{
    m_selected_drawable = nullptr;
    for(auto v : m_converted_nodes)
    {
        m_scene->RemoveChild(v.second);
    }
    m_converted_nodes.clear();
    m_coh_scene.reset(new CoHSceneGraph);
    loadSceneGraph(*m_coh_scene,path);
    emit scenegraphLoaded(*m_coh_scene);
}

void MapViewerApp::onNodeSelected(CoHNode * n)
{
    m_current_selected_node = n;
}

#define MAX_GRAPH_DEPTH 80

int created_node_count = 0;
void MapViewerApp::onDisplayRef(ConvertedRootNode *root,bool show_all)
{
    if(nullptr==root)
    {
        if (m_currently_shown_node)
            m_currently_shown_node->SetEnabledRecursive(false);
        m_currently_shown_node = nullptr;
        return;
    }
    auto  iter = m_converted_nodes.find(root->node);
    Node *boxNode;
    if (iter == m_converted_nodes.end())
    {
        Urho3D::Matrix3x4 fromglm;
        fromglm.m00_ = root->mat[0][0];
        fromglm.m10_ = root->mat[0][1];
        fromglm.m20_ = root->mat[0][2];
        fromglm.m01_ = root->mat[1][0];
        fromglm.m11_ = root->mat[1][1];
        fromglm.m21_ = root->mat[1][2];
        fromglm.m02_ = root->mat[2][0];
        fromglm.m12_ = root->mat[2][1];
        fromglm.m22_ = root->mat[2][2];
        fromglm.m03_ = root->mat[3][0];
        fromglm.m13_ = root->mat[3][1];
        fromglm.m23_ = root->mat[3][2];

        boxNode = convertedNodeToLutefisk(root->node, fromglm, m_context, MAX_GRAPH_DEPTH,
                                          show_all ? CONVERT_EDITOR_MARKERS : CONVERT_MINIMAL);
        m_scene->AddChild(boxNode);
        m_converted_nodes[root->node] = boxNode;
    }
    else
        boxNode = iter->second;
    boxNode->SetEnabledRecursive(!boxNode->IsEnabled());
    m_currently_shown_node = boxNode;
}
void MapViewerApp::onDisplayNode(CoHNode *n,bool rootnode)
{
    if(nullptr==n)
    {
        if (m_currently_shown_node)
            m_currently_shown_node->SetEnabledRecursive(false);
        m_currently_shown_node = nullptr;
        return;
    }
    auto  iter = m_converted_nodes.find(n);
    Node *boxNode;
    if (iter == m_converted_nodes.end())
    {
        boxNode = convertedNodeToLutefisk(n, Matrix3x4::IDENTITY, m_context, MAX_GRAPH_DEPTH,rootnode ? CONVERT_MINIMAL : CONVERT_EDITOR_MARKERS);
        m_scene->AddChild(boxNode);
        m_converted_nodes[n] = boxNode;
    }
    else
        boxNode = iter->second;
    // Look at selected node
    if(boxNode!=nullptr)
    {
        m_camera_node->LookAt(boxNode->GetWorldPosition());
        Vector3 nodecenter=n->m_bbox.Center();
        Vector3 dir = (m_camera_node->GetWorldPosition() - nodecenter).Normalized();
        Vector3 newpos = nodecenter + dir*n->m_bbox.size().Length()*1.5f;
        //m_camera_node->SetPosition(newpos);
        Quaternion camrot=m_camera_node->GetRotation();
        pitch_=camrot.PitchAngle();
        yaw_=camrot.YawAngle();
    }
    if(boxNode==m_currently_shown_node || nullptr==boxNode)
        return;
    if (m_currently_shown_node)
        m_currently_shown_node->SetEnabledRecursive(false);
    boxNode->SetEnabledRecursive(true);
    m_currently_shown_node = boxNode;

}

void MapViewerApp::HandleKeyUp(int key,int scancode,unsigned buttons,int qualifiers)
{
    // Close console (if open) or exit when ESC is pressed
    if (key == KEY_ESCAPE)
    {
        engine_->Exit();
    }
    if(key>='1' && key<='8') {
        int layernum = key-'1';
        if(layernum>=m_coh_scene->refs.size())
            return;
        auto ref_to_toggle = m_coh_scene->refs[layernum];
        Node *v = m_converted_nodes[ref_to_toggle];
        if(v)
            v->SetDeepEnabled(!v->IsEnabled());
    }
}

void MapViewerApp::HandleKeyDown(int key,int scancode,unsigned buttons,int qualifiers, bool repeat)
{
    // Toggle console with F1
    if (key == KEY_F1)
        m_context->GetSubsystemT<Console>()->Toggle();

    // Toggle debug HUD with F2
    else if (key == KEY_F2)
        m_context->GetSubsystemT<DebugHud>()->ToggleAll();
    if (key == '9')
    {
        Graphics* graphics = m_context->m_Graphics.get();
        Image screenshot(m_context);
        graphics->TakeScreenShot(screenshot);
        // Here we save in the Data folder with date and time appended
        screenshot.SavePNG("Data/Screenshot_" +
                           Time::GetTimeStamp().replace(':', '_').replace('.', '_').replace(' ', '_') + ".png");
    }
}

bool MapViewerApp::Raycast(float maxDistance)
{
    Vector3 hitPos;
    Drawable* hitDrawable = nullptr;

    UI* ui = m_context->m_UISystem.get();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (!ui->GetCursor()->IsVisible() || ui->GetElementAt(pos, true))
        return false;

    Graphics* graphics = m_context->m_Graphics.get();
    Camera* camera = m_camera_node->GetComponent<Camera>();
    Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    std::vector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    m_scene->GetComponent<Octree>()->RaycastSingle(query);
    if (!results.empty())
    {
        RayQueryResult& result = results[0];
        hitPos = result.position_;
        hitDrawable = result.drawable_;
        Variant stored = result.node_->GetVar("CoHModel");
        Variant stored_node = result.node_->GetVar("CoHNode");
        if(stored!=Variant::EMPTY)
        {
            m_selected_drawable = hitDrawable;
            emit modelSelected((CoHNode *)stored_node.GetVoidPtr(),(CoHModel *)stored.GetVoidPtr(),hitDrawable);
        }
        return true;
    }
    m_selected_drawable = nullptr;
    emit modelSelected(nullptr,nullptr,nullptr);
    return false;
}

void MapViewerApp::HandleUpdate(float timeStep)
{
    Input* input = m_context->m_InputSystem.get();
    qApp->processEvents();
    UI* ui = m_context->m_UISystem.get();
    ui->GetCursor()->SetVisible(!input->GetMouseButtonDown(MouseButton::RIGHT));
    input->SetMouseMode(ui->GetCursor()->IsVisible() ? MM_FREE : MM_RELATIVE);//,MM_RELATIVE

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
    if(shifted)
        timeStep *=10;
    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    bool cameraLocationUpdated = false;
    if (input->GetKeyDown(KEY_W)) {
        m_camera_node->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_S)) {
        m_camera_node->Translate(Vector3::BACK * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_A)) {
        m_camera_node->Translate(Vector3::LEFT * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_D)) {
        m_camera_node->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep * 2);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_PAGE_UP) || input->GetKeyDown(KEY_Q)) {
        m_camera_node->Translate(Vector3::UP * MOVE_SPEED * timeStep * 5);
        cameraLocationUpdated = true;
    }
    if (input->GetKeyDown(KEY_PAGE_DOWN) || input->GetKeyDown(KEY_E)) {
        m_camera_node->Translate(Vector3::DOWN * MOVE_SPEED * timeStep * 5);
        cameraLocationUpdated = true;
    }
    if(cameraLocationUpdated) {
        Vector3 pos=m_camera_node->GetPosition();
        emit cameraLocationChanged(pos.x_,pos.y_,pos.z_);
    }
    if (ui->GetCursor()->IsVisible() && input->GetMouseButtonPress(MouseButton::LEFT))
        Raycast(8500);
}

void MapViewerApp::HandlePostRenderUpdate(float ts)
{
    // If draw debug mode is enabled, draw viewport debug geometry. Disable depth test so that we can see the effect of occlusion
    if (m_selected_drawable) {
        m_selected_drawable->DrawDebugGeometry(m_scene->GetComponent<DebugRenderer>(),true);
    }
    if(m_current_selected_node) {
        BoundingBox bbox(m_current_selected_node->m_bbox);
        auto weak_node(m_current_selected_node->m_lutefisk_result);
        if(weak_node)
            bbox.Transform(weak_node->GetWorldTransform());
        m_scene->GetComponent<DebugRenderer>()->AddBoundingBox(bbox,Color::BLUE,false);
    }
}

//! @}
