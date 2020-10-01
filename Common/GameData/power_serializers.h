/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>

class BinStore;
class QString;
struct AllPowerCategories;

static constexpr uint32_t powers_i0_requiredCrc=0xA53BBC94;
bool loadFrom(BinStore *s, AllPowerCategories &target);
bool loadFrom(const QString &filepath, AllPowerCategories &target);
void saveTo(const AllPowerCategories &target,const QString &baseName,bool text_format=false);
