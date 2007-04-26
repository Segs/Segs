/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: PacketUtil.cpp 253 2006-08-31 22:00:14Z malign $
 */

#include "PacketUtil.h"
PacketCollector::~PacketCollector()
{
	for_each(avail_packets.begin(),avail_packets.end(),&PacketCollector::PacketDestroyer);
	for_each(sibling_map.begin(),sibling_map.end(),&PacketCollector::PacketSibDestroyer);
}
