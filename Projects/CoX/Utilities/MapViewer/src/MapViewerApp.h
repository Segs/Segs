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
struct ConvertedSceneGraph;
struct ConvertedNode;
struct ConvertedModel;

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
signals:
    void cameraLocationChanged(float x,float y,float z);
    void nodeSelected(ConvertedNode *def,Urho3D::Node *n);
    void modelSelected(ConvertedModel *model,Urho3D::Drawable*m);
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
    Urho3D::SharedPtr<Urho3D::Scene> m_scene;
    // Camera params
    float yaw_=0;
    float pitch_=0;
    std::unique_ptr<ConvertedSceneGraph> m_coh_scene;
    std::unordered_map<void *,Urho3D::Node *> m_converted_nodes;
    SideWindow *m_sidewindow=nullptr;
    Urho3D::Drawable *m_selected_drawable=nullptr;
};

#endif // MAPVIEWERAPP_H
