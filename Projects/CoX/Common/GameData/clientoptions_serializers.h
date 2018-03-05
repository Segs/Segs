/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <stdint.h>

class QString;
class ClientOptions;

template<class Archive>
void serialize(Archive & archive, ClientOptions &cd, uint32_t const version);

void saveTo(const ClientOptions &target,const QString &baseName,bool text_format=false);

void serializeToDb(const ClientOptions &data, QString &tgt);
void serializeFromDb(ClientOptions &data, const QString &src);
