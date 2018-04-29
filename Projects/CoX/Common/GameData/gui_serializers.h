/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <stdint.h>

class QString;
class GUIWindow;
class GUISettings;

template<class Archive>
void serialize(Archive &archive, GUIWindow &wnd);
template<class Archive>
void serialize(Archive &archive, GUISettings &gui, uint32_t const version);

void saveTo(const GUISettings &target,const QString &baseName,bool text_format=false);

void serializeToDb(const GUISettings &data, QString &tgt);
void serializeFromDb(GUISettings &data, const QString &src);
