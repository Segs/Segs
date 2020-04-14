/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>
#include <vector>

class BinStore;
class QString;

typedef std::vector<struct Map_Data> AllMaps_Data;

static constexpr uint32_t zones_i0_requiredCrc=0x520EE128;
bool loadFrom(BinStore *s,AllMaps_Data &target);
void saveTo(const AllMaps_Data &target,const QString &baseName,bool text_format=false);
