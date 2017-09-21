#pragma once

#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Math/BoundingBox.h>

#include <vector>
#include <QString>
#include <QHash>

namespace Urho3D {
class Node;
class Context;
};

enum class CoHBlendMode : uint8_t
{
    MULTIPLY                = 0,
    MULTIPLY_REG            = 1,
    COLORBLEND_DUAL         = 2,
    ADDGLOW                 = 3,
    ALPHADETAIL             = 4,
    BUMPMAP_MULTIPLY        = 5,
    BUMPMAP_COLORBLEND_DUAL = 6,
    INVALID                 = 255,
};
namespace Urho3D {
    class Texture;
}

