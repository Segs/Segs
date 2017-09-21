#ifndef MAPVIEWERAPP_H
#define MAPVIEWERAPP_H
#include "Lutefisk3D/Engine/Application.h"
#include <unordered_map>
#include <memory>
#include <QObject>

class SideWindow; // Qt side window
namespace Urho3D {
class Node;
class Scene;
class StaticModel;
class Vector3;
class Drawable;
}
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
signals:
    void cameraLocationChanged(float x,float y,float z);
private:
    void CreateBaseScene();
    void SetupViewport();
    void CreateConsoleAndDebugHud();
    void prepareSideWindow();
    void prepareCursor();
    void HandlePostRenderUpdate(float ts);
    void HandleKeyUp(int key, int scancode, unsigned buttons, int qualifiers);
    void HandleKeyDown(int key, int scancode, unsigned buttons, int qualifiers, bool repeat);
    void HandleUpdate(float ts);

    Urho3D::SharedPtr<Urho3D::Node> m_camera_node;
    Urho3D::SharedPtr<Urho3D::Scene> m_scene;
    // Camera params
    float yaw_=0;
    float pitch_=0;
    SideWindow *m_sidewindow=nullptr;

};

#endif // MAPVIEWERAPP_H
