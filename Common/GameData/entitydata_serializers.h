/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
