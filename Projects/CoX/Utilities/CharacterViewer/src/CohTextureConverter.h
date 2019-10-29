/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/Model.h"

#include <Lutefisk3D/Container/Ptr.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/Vector2.h>
#include <glm/vec2.hpp>
#include <QHash>
#include <vector>
#include <unordered_map>

class QString;

namespace Urho3D
{
    class Texture;
    class Context;
}

extern std::unordered_map<uint32_t,Urho3D::SharedPtr<Urho3D::Texture>> g_converted_textures;

SEGS::HTexture tryLoadTexture(Urho3D::Context *ctx, const QString &fname);
std::vector<SEGS::HTexture> getModelTextures(Urho3D::Context *ctx,std::vector<QString> &a1);
