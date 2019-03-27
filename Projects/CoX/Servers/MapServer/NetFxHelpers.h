/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Handle.h"

#include <glm/vec3.hpp>

struct NetFx;
class QString;
class Entity;

using NetFxHandle = HandleT<NetFx>;

NetFxHandle createNetFx(const QString &fx_name);
void release(NetFxHandle handle);
NetFx &lookup(NetFxHandle handle);
void setSourceLocation(NetFxHandle h,glm::vec3 loc);
void setSourceEntityAndBone(NetFxHandle h,int entidx,uint8_t boneidx);
void setTargetLocation(NetFxHandle h,glm::vec3 loc);

void attachToEntity(NetFxHandle h, Entity *tgt);
