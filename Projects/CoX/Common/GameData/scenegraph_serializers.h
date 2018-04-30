/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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
//! Generic loader function will load cereal version, or if that does not exists a bin version
bool LoadSceneData(const QString &fname, SceneGraph_Data &scenegraph);
