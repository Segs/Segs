/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
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
