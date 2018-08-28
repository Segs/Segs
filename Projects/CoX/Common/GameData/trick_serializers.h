/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "trick_definitions.h"
class BinStore;
class QString;

constexpr const static uint32_t tricks_i0_requiredCrc = 0xB46B669E;
constexpr const static uint32_t tricks_i2_requiredCrc = 0xB06B1696;
bool loadFrom(BinStore *s,AllTricks_Data *target);
bool LoadModifiersData(const QString &fname, AllTricks_Data &m);
void saveTo(const AllTricks_Data &target,const QString &baseName,bool text_format=false);
