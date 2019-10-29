/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <string>
#include <stdint.h>
class QString;
class ACE_INET_Addr;

bool parseAddress(const QString &src,ACE_INET_Addr &tgt);
