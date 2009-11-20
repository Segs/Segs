/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapEvents.h"

MapLinkEvent *MapEventFactory::EventFromStream(BitStream &bs)
{
    MapLinkEvent *ev=0;
    size_t read_pos = bs.GetReadPos();

    ev = CRUD_EventFactory<MapLinkEvent>::EventFromStream(bs);
    if(ev) // base class created the event
    {
        return ev;
    }
    bs.SetReadPos(read_pos); // rewind the stream
    u8 opcode = bs.GetPackedBits(1);
    switch(opcode)
    {
    case 1:
        ev = new ConnectRequest<MapLinkEvent>; break;
    case 9:
        ev = new NewEntity; 
        break;
    case 2:
        ev = new InputState;
        break;
    case 4:
        ev = new ShortcutsRequest;
        break;
    case 3:
        ev = new SceneRequest;
        break;
    case 5:
        ev = new EntitiesRequest;
        break;
    default:
        ev = new MapUnknownRequest;
    }
    return ev;
}
