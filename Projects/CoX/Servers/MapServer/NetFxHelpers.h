/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/Runtime/Handle.h"

#include <glm/vec3.hpp>

struct NetFx;
class QString;
class Entity;
namespace FXSystem
{
struct Data;
}

using NetFxHandle = HandleT<20,12,NetFx>;
using FxHandle = HandleT<20,12,FXSystem::Data>;

NetFxHandle createNetFx(FxHandle from_fx);
void release(NetFxHandle handle);
NetFx &lookup(NetFxHandle handle);
void attachToEntity(NetFxHandle h, Entity *tgt);
void updateNetFx();
bool updateNetFxFromParent(NetFxHandle h);
