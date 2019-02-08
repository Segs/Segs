/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>
#include <vector>

class BinStore;
class QString;
struct SceneGraph_Data;

static constexpr uint32_t scenegraph_i0_2_requiredCrc=0xD3432007;
bool loadFrom(BinStore *s,SceneGraph_Data &target);
bool loadFrom(const QString &filepath,SceneGraph_Data &target);
void saveTo(const SceneGraph_Data &target,const QString &baseName,bool text_format=false);
QString getFilepathCaseInsensitive(QString fpath);
//! Generic loader function will load cereal version, or if that does not exists a bin version
bool LoadSceneData(const QString &fname, SceneGraph_Data &scenegraph);
