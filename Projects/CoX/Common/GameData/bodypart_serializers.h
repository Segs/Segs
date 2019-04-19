/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/bodypart_definitions.h"
#include <cstdint>
#include <vector>

class BinStore;
class QString;
struct BodyPart_Data;

using AllBodyParts_Data = std::vector<BodyPart_Data>;

constexpr const static uint32_t bodyparts_i0_requiredCrc = 0x541B59EB;
bool loadFrom(BinStore *s,AllBodyParts_Data *target) ;
void saveTo(const AllBodyParts_Data &target,const QString &baseName,bool text_format=false);

