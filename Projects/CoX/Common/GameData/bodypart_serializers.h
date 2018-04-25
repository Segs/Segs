/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <stdint.h>
#include <vector>
#include "Common/GameData/bodypart_definitions.h"
class BinStore;
class QString;
struct BodyPart_Data;

typedef std::vector<BodyPart_Data> AllBodyParts_Data;

constexpr const static uint32_t bodyparts_i0_requiredCrc = 0x541B59EB;
bool loadFrom(BinStore *s,AllBodyParts_Data *target) ;
void saveTo(const AllBodyParts_Data &target,const QString &baseName,bool text_format=false);

