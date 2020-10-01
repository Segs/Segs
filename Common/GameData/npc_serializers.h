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
using AllNpcs_Data = std::vector<struct Parse_NPC>;

static constexpr uint32_t npccostumesets_i0_requiredCrc=0x944EC021;
bool loadFrom(BinStore * s, AllNpcs_Data & target);
void saveTo(const AllNpcs_Data &target,const QString &baseName,bool text_format=false);
