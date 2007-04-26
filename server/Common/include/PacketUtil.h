/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: PacketUtil.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include <assert.h>
#include <deque>
#include <algorithm>

#include <map>
#include <set>
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
using namespace stdext;
#endif
#include "Packet.h"
using namespace std;
// Net class pushes packets into the collector objects
