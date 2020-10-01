/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameServer Projects/CoX/Servers/GameServer
 * @{
 */

#include "GameEventFactory.h"
#include "Messages/Game/GameEvents.h"

using namespace SEGSEvents;

GameLinkEvent *GameEventFactory::EventFromStream(BitStream &bs)
{
    size_t read_pos = bs.GetReadPos();
    //size_t bits_avail = bs.GetReadableBits();
    GameLinkEvent *ev = CRUD_EventFactory::EventFromStream(bs);
    if(ev) // base class created the event
        return ev;
    bs.SetReadPos(read_pos); // otherwise rewind the stream and retry

    int32_t opcode = bs.GetPackedBits(1);
    switch(opcode)
    {
    case 1: return new ConnectRequest;
    case 2: return new UpdateServer;
    case 3: return new MapServerAddrRequest;
    case 4: return new DeleteCharacter;
    case 5: return new UpdateCharacter;
    default: return new UnknownEvent;
    }
    return ev;
}

//! @}
