/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <string>
#include <stdint.h>
class QString;
class ACE_INET_Addr;

bool parseAddress(const QString &src,ACE_INET_Addr &tgt);
