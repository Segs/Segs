/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Cmsg_DB_Hello.h 253 2006-08-31 22:00:14Z malign $
 */

#include "Base.h"
#include "Packet.h"

/*
Valid packet: B1 00 00 00 41 1C 05 F0 02 00 00 00 00 F9 FF FF FF 0B 0A 59 60 1C 00 00 
*/

class Cmsg_DB_Hello : public Packet
{
public:
	void serializeto()
	{
		StorePackedBits(1, 1);
		StorePackedBits(1, 0x18174393);
		StorePackedBits(1, 4);
	}
};
