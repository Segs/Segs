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

struct Parse_CharAttrib;
struct Parse_CharAttribMax;
struct AttribNames_Data;

constexpr const static uint32_t attribnames_i0_requiredCrc = 0xED2ECE38;
bool loadFrom(BinStore *s,AttribNames_Data &target) ;
void saveTo(const AttribNames_Data &target,const QString &baseName,bool text_format=false);

bool loadFrom(BinStore *s, Parse_CharAttrib &target);
bool loadFrom(BinStore *s, Parse_CharAttribMax &target);

template<class Archive>
void serialize(Archive & archive, Parse_CharAttrib & m);
template<class Archive>
void serialize(Archive & archive, Parse_CharAttribMax & m);
