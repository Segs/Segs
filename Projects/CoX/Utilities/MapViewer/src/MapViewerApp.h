/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/SceneGraph.h"
#include "Lutefisk3D/Engine/Application.h"
#include <unordered_map>
#include <memory>
#include <QObject>

class SideWindow; // Qt side window

namespace Urho3D
{
    class Node;
    class Scene;
    class StaticModel;
    class Vector3;
    class Drawable;
}


using CoHNode = SEGS::SceneNode;
using CoHModel = SEGS::Model;

class MapViewerApp : public QObject, public Urho3D::Application
{
    Q_OBJECT
public:
    MapViewerApp(Urho3D::Context *ctx);
    ~MapViewerApp() override;
    // Application interface
public:
    void Setup() override;
    void Start() override;
public slots:
    void loadSelectedSceneGraph(const QString &path);
    void onDisplayNode(SEGS::SceneNode *n, bool rootnode);
    void onDisplayRef(SEGS::RootNode *root, bool show_all);
    void onNodeSelected(SEGS::SceneNode *n);
signals:
    void cameraLocationChanged(float x,float y,float z);
    void nodeSelected(SEGS::SceneNode *def,Urho3D::Node *n);
    void modelSelected(SEGS::SceneNode *def,SEGS::Model *model,Urho3D::Drawable*m);
    void scenegraphLoaded(const SEGS::SceneGraph &);
private:
    void CreateBaseScene();
    void SetupViewport();
    void CreateConsoleAndDebugHud();
    void prepareSideWindow();
    void prepareCursor();
    bool Raycast(float maxDistance);
    void HandlePostRenderUpdate(float ts);
    void HandleKeyUp(int key, int scancode, unsigned buttons, int qualifiers);
    void HandleKeyDown(int key, int scancode, unsigned buttons, int qualifiers, bool repeat);
    void HandleUpdate(float ts);

    Urho3D::SharedPtr<Urho3D::Node> m_camera_node;
    Urho3D::SharedPtr<Urho3D::Node> m_currently_shown_node;
    SEGS::SceneNode *m_current_selected_node=nullptr;
    Urho3D::SharedPtr<Urho3D::Scene> m_scene;
    // Camera params
    float yaw_=0;
    float pitch_=0;
    // conversion results
    std::unique_ptr<SEGS::SceneGraph> m_coh_scene;
    std::unordered_map<void *,Urho3D::Node *> m_converted_nodes;
    // UI helper variables
    SideWindow *m_sidewindow=nullptr;
    Urho3D::Drawable *m_selected_drawable=nullptr;
};
