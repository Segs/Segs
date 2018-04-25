/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <stdint.h>

class BinStore;
class QString;
struct AllPowerCategories;

static constexpr uint32_t powers_i0_requiredCrc=0xA53BBC94;
bool loadFrom(BinStore *s, AllPowerCategories &target);
void saveTo(const AllPowerCategories &target,const QString &baseName,bool text_format=false);
