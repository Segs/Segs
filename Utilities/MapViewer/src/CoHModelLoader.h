/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CohTextureConverter.h"
#include "Common/Runtime/Model.h"
#include <glm/vec3.hpp>
#include <QStringList>
#include <memory>
#include <vector>
#include <stdint.h>
namespace Urho3D
{
    class StaticModel;
}
namespace SEGS 
{
    struct VBOPointers;
}
using CoHModel = SEGS::Model;

float *combineBuffers(SEGS::VBOPointers &meshdata, SEGS::Model *mdl);
