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
private:
    SideWindow *m_sidewindow=nullptr;

};

#endif // MAPVIEWERAPP_H
