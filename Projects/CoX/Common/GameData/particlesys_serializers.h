#pragma once

#include <stdint.h>

class BinStore;
class QString;
constexpr const static uint32_t particlesystems_i0_requiredCrc = 0x46714C4E;
bool loadFrom(BinStore *s,struct Parse_AllPSystems *target) ;
void saveTo(const Parse_AllPSystems &target,const QString &baseName,bool text_format=false);


