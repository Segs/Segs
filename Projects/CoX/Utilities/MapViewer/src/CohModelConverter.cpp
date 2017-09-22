#include "CohModelConverter.h"
#include "CoHSceneConverter.h"
#include "CohTextureConverter.h"

#include <Lutefisk3D/Scene/Node.h>

#include <Lutefisk3D/Core/Context.h>
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
#include <GameData/trick_definitions.h>
extern QString basepath;
using namespace Urho3D;
Urho3D::StaticModel *convertToLutefiskModel(Urho3D::Context *ctx, Urho3D::Node *tgtnode, ConvertedNode *def) 
{
    return nullptr;
}
