/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>

class QString;
class GUIWindow;
class GUISettings;

template<class Archive>
void serialize(Archive &archive, GUISettings &gui, uint32_t const version);

void saveTo(const GUISettings &target,const QString &baseName,bool text_format=false);

void serializeToDb(const GUISettings &data, QString &tgt);
void serializeFromDb(GUISettings &data, const QString &src);
