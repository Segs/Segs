/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <stdint.h>

class QString;
struct EntityData;

template<class Archive>
void serialize(Archive & archive, EntityData & m, uint32_t const version);

void saveTo(const EntityData &target,const QString &baseName,bool text_format=false);

void serializeToDb(const EntityData &data, QString &tgt);
void serializeFromDb(EntityData &data, const QString &src);
