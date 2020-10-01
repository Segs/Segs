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

struct Parse_CharAttrib;
struct Parse_CharAttribMax;
struct AttribNames_Data;

constexpr const static uint32_t attribnames_i0_requiredCrc = 0xED2ECE38;
bool loadFrom(BinStore *s,AttribNames_Data &target) ;
void saveTo(const AttribNames_Data &target,const QString &baseName,bool text_format=false);

bool loadFrom(BinStore *s, Parse_CharAttrib &target);
bool loadFrom(BinStore *s, Parse_CharAttribMax &target);

void serializeToDb(const Parse_CharAttrib &data, QString &tgt);
void serializeFromDb(Parse_CharAttrib &data, const QString &src);
