#pragma once
#include <stdint.h>
#include <vector>

class BinStore;
class QString;

typedef std::vector<struct Map_Data> AllMaps_Data;

static constexpr uint32_t zones_i0_requiredCrc=0x520EE128;
bool loadFrom(BinStore *s,AllMaps_Data &target);
void saveTo(const AllMaps_Data &target,const QString &baseName,bool text_format=false);
