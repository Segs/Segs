#ifndef MAPVIEWERAPP_H
#define MAPVIEWERAPP_H
#include "Lutefisk3D/Engine/Application.h"
#include <unordered_map>
#include <memory>
#include <QObject>

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
};

#endif // MAPVIEWERAPP_H
