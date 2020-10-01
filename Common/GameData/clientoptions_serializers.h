/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>

class QString;
class ClientOptions;

void saveTo(const ClientOptions &target,const QString &baseName,bool text_format=false);
void serializeToDb(const ClientOptions &data, QString &tgt);
void serializeFromDb(ClientOptions &data, const QString &src);
