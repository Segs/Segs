/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
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

    uint16_t opcode = bs.GetPackedBits(1);

    switch(opcode) // this is the actual clientside packet Opcode
    {
        case 1: return new ConnectRequest;
        case 2: return new InputState;
        case 3: return new SceneRequest;
        case 4: return new ShortcutsRequest;
        case 5: return new EntitiesRequest;
        case 6: return new CookieRequest;
        case 7: return new ClientQuit;
        case 9: return new NewEntity;
            //   default: return new MapUnknownRequest;
    }
    ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled event type %d\n"),opcode));
    return nullptr;
}

MapLinkEvent *MapEventFactory::CommandEventFromStream(BitStream & bs)
{
    uint16_t opcode = bs.GetPackedBits(1);

    switch(opcode)
    {
        case 0:
            if(bs.GetReadableBits()>=8) // at least 1 char readable ?
                return new ConsoleCommand;
            // otherwise treat as idle
            return new IdleEvent;
        case 1: return new MiniMapState;
        case 4: return new ClientResumedRendering;
        case 14: return new WindowState;
        case 40: return new CombineRequest;
        /*case 21: new KeybindProfileReset; */
        /*case 22: new KeybindProfileSelected; */
        /*case 37: new TargetChatChannelSelected; */
    }
    ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled command event type %d\n"),opcode));
    return nullptr;

}
