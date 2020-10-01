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
struct ColorFx;
using Fx_AllBehaviors = std::vector<struct FxBehavior>;
using Fx_AllInfos = std::vector<struct FxInfo>;

constexpr const static uint32_t fxbehaviors_i0_requiredCrc = 0x0DD5777C;
bool loadFrom(BinStore *s,Fx_AllBehaviors &target) ;
bool LoadFxBehaviorData(const QString &fname, Fx_AllBehaviors &behaviors);
void saveTo(const Fx_AllBehaviors &target,const QString &baseName,bool text_format=false);

constexpr const static uint32_t fxinfos_i0_requiredCrc = 0xB178A55D;
bool loadFrom(BinStore *s,Fx_AllInfos &target);
bool LoadFxInfoData(const QString &fname, Fx_AllInfos &infos);
void saveTo(const Fx_AllInfos &target,const QString &baseName,bool text_format=false);

template<class Archive>
void serialize(Archive & archive, ColorFx & m);
