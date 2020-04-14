/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>
#include <vector>

class QString;
class BinStore;
struct LevelExpAndDebt;
struct Parse_Combining;
struct Parse_Effectiveness;
struct Parse_PI_Schedule;
typedef std::vector<struct Parse_Origin> Parse_AllOrigins;

constexpr const static uint32_t levelsdebts_i0_requiredCrc = 0x8F0D1A87;
bool loadFrom(BinStore *s,LevelExpAndDebt &target);
void saveTo(const LevelExpAndDebt & target, const QString &baseName, bool text_format=false);

constexpr const static uint32_t combining_i0_requiredCrc = 0x7F06A6D1;
bool loadFrom(BinStore *s,Parse_Combining &target);
void saveTo(const Parse_Combining & target, const QString &baseName, bool text_format=false);


constexpr const static uint32_t boosteffectiveness_i0_requiredCrc = 0x5706CEC8;
bool loadFrom(BinStore *s,Parse_Effectiveness &target);
void saveTo(const Parse_Effectiveness & target, const QString &baseName, bool text_format=false);


constexpr const static uint32_t origins_i0_requiredCrc = 0x763EA46E;
bool loadFrom(BinStore *s,Parse_AllOrigins &target);
void saveTo(const Parse_AllOrigins & target, const QString &baseName, bool text_format=false);


constexpr const static uint32_t pischedule_i0_requiredCrc = 0x27267F27;
bool loadFrom(BinStore *s,Parse_PI_Schedule &target);
void saveTo(const Parse_PI_Schedule & target, const QString &baseName, bool text_format=false);
