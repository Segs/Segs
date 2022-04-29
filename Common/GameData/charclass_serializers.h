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
using Parse_AllCharClasses = std::vector<struct CharClass_Data>;

constexpr const static uint32_t charclass_i0_requiredCrc = 0xF6D734A6;
constexpr const static uint32_t charclass_i24_requiredCrc = 0x7FF68AAF;
bool loadFrom(BinStore *s,Parse_AllCharClasses &target) ;
void saveTo(const Parse_AllCharClasses &target,const QString &baseName,bool text_format=false);
