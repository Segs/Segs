/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/Powers.h"

#include <stdint.h>

class QString;
class PowerTrayItem;
class PowerTray;
class PowerTrayGroup;

void saveTo(const PowerTrayGroup &target, const QString &baseName, bool text_format=false);

void serializeToDb(const PowerTrayGroup &data, QString &tgt);
void serializeFromDb(PowerTrayGroup &data, const QString &src);
