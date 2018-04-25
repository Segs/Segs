/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <string>
#include <stdint.h>
class QString;
class ACE_INET_Addr;

bool parseAddress(const QString &src,ACE_INET_Addr &tgt);
