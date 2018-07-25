/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/NetStructures/Powers.h"

#include <stdint.h>

class QString;
class Power;
class PowerTray;
class PowerTrayGroup;

template<class Archive>
void serialize(Archive &archive, Power &pwr, uint32_t const version);
template<class Archive>
void serialize(Archive &archive, PowerTray &ptray, uint32_t const version);
template<class Archive>
void serialize(Archive &archive, PowerTrayGroup &ptraygroup, uint32_t const version);

void saveTo(const PowerTrayGroup &target, const QString &baseName, bool text_format=false);

void serializeToDb(const PowerTrayGroup &data, QString &tgt);
void serializeFromDb(PowerTrayGroup &data, const QString &src);
