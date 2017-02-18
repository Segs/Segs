/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "MapEvents.h"
void processCommon(int op,BitStream &bs) {
    size_t read_pos = bs.GetReadPos();
    switch(op) {
        case 110:
            ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Spawn if dead ? \n")));
            break;
        case 104:
            ACE_DEBUG ((LM_WARNING,ACE_TEXT ("G-Packed ? \n")));
            break;
        case 100: {
            QString cmd;
            bs.GetString(cmd);
            printf("DevConsole command %s\n",qPrintable(cmd));
            break;
        }
        case 102: {
            int sz = bs.GetReadableBits()/8;
            if(sz!=0) {
            FILE *fp = fopen("dump.bin","wb");

            uint8_t *g = new uint8_t(sz);
            bs.GetBitArray(g,bs.GetReadableBits());
            fwrite(g,sz,1,fp);
            fclose(fp);
            exit(0);
            }
        }
            break;
    }
}
MapLinkEvent *MapEventFactory::EventFromStream(BitStream &bs,bool follower)
{
    if(!follower) {
        size_t read_pos = bs.GetReadPos();

        MapLinkEvent *ev = CRUD_EventFactory::EventFromStream(bs);
        if(ev) // base class created the event
            return ev;

        bs.SetReadPos(read_pos); // rewind the stream and retry
    }
    uint16_t opcode = bs.GetPackedBits(1);
    if(follower)
        opcode+=100;

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
        case 100:
            if(bs.GetReadableBits()>=8) // at least 1 char readable ?
                return new ConsoleCommand;
            // otherwise treat as idle
            return new IdleEvent;
        case 101: return new MiniMapState;
        case 114: return new WindowState;
        case 40: return new CombineRequest;
            //   default: return new MapUnknownRequest;
    }
    if(opcode>=100) {
        processCommon(opcode,bs);
        return (MapLinkEvent *)1;
    }
    ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Unhandled event type %d\n"),opcode));
    return 0;
}
