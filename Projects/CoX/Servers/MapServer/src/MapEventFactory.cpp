/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#include "MapEvents.h"

MapLinkEvent *MapEventFactory::EventFromStream(BitStream &bs)
{
    size_t read_pos = bs.GetReadPos();

    MapLinkEvent *ev = CRUD_EventFactory::EventFromStream(bs);
    if(ev) // base class created the event
        return ev;

    bs.SetReadPos(read_pos); // rewind the stream and retry
    uint8_t opcode = bs.GetPackedBits(1);
    switch(opcode) // this is the actual clientside packet Opcode
    {
    case 1: return new ConnectRequest;
    case 9: return new NewEntity;
    case 2: return new InputState;
    case 4: return new ShortcutsRequest;
    case 3: return new SceneRequest;
    case 5: return new EntitiesRequest;
    case 40: return new CombineRequest;
    default: return new MapUnknownRequest;
    }
    return 0;
}
