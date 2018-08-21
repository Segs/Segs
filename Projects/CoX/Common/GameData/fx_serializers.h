#pragma once

#include <stdint.h>
#include <vector>
class BinStore;
class QString;

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

