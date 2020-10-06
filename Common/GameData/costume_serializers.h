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
struct Costume2_Data;
struct Pallette_Data;
struct GeoSet_Data;
struct TailorCost_Data;
typedef std::vector<TailorCost_Data> AllTailorCosts_Data;
typedef std::vector<Costume2_Data> CostumeSet_Data;

static constexpr uint32_t tailorcosts_i0_requiredCrc=0xEC2F43E6;
bool loadFrom(BinStore * s, AllTailorCosts_Data * target);
void saveTo(const AllTailorCosts_Data &target,const QString &baseName,bool text_format=false);

static constexpr uint32_t costumesets_i0_requiredCrc=0x9A79F6D5;
bool loadFrom(BinStore * s, CostumeSet_Data * target);
void saveTo(const CostumeSet_Data &target,const QString &baseName,bool text_format=false);

static constexpr uint32_t palette_i0_requiredCrc=0x3C075513;
bool loadFrom(BinStore *s,Pallette_Data *target);
void saveTo(const Pallette_Data &target,const QString &baseName,bool text_format=false);

static constexpr uint32_t geoset_i0_requiredCrc=0xB232BD7F;
bool loadFrom(BinStore *s,GeoSet_Data *target);
void saveTo(const GeoSet_Data &target,const QString &baseName,bool text_format=false);
