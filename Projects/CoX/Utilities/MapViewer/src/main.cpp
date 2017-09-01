#include "Lutefisk3D/Engine/Engine.h"
#include "Lutefisk3D/Core/Context.h"
#include "Lutefisk3D/Engine/Application.h"

#include "MapViewerApp.h"

#include <memory>
using namespace Urho3D;
int main(int argc,char **argv)
{
    Urho3D::ParseArguments(argc, argv);
    auto context = std::make_shared<Context>();
    MapViewerApp app(context.get());
    app.Run();
    return 0;
}
