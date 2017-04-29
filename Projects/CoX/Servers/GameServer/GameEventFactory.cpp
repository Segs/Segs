#include "GameEventFactory.h"
#include "GameEvents.h"
GameLinkEvent *GameEventFactory::EventFromStream(BitStream &bs, bool followe)
{
    GameLinkEvent *ev;

    size_t read_pos = bs.GetReadPos();
    //size_t bits_avail = bs.GetReadableBits();
    ev = CRUD_EventFactory::EventFromStream(bs);
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
    default: return new GameUnknownRequest;
    }
    return ev;
}

