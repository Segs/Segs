/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
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
    qCWarning(logMapEvents, "Unhandled event type %d", opcode);
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
        //case 2: return new Unknown2; // TODO: What is this?
        //case 3: return new RefreshWindows; // TODO: Refreshes all windows? Issue #268
        case 4: return new ClientResumedRendering;
        case 7: return new InteractWithEntity;
        case 8: return new SwitchTray;
        case 9: return new EnterDoor;
        case 11: return new SetDestination;
        case 14: return new WindowState;
        case 16: return new ChatDividerMoved;
        case 17: return new InspirationDockMode;
        case 18: return new PowersDockMode;
        case 19: return new SetKeybind;
        case 20: return new RemoveKeybind;
        case 21: return new ResetKeybinds;
        case 22: return new SelectKeybindProfile;
        case 29: return new ActivateInspiration;
        case 30: return new SetDefaultPowerSend;
        case 31: return new SetDefaultPower;
        case 32: return new UnqueueAll;
        case 33: return new AbortQueuedPower;
        case 36: return new ChangeStance;
        case 37: return new TargetChatChannelSelected;
        case 38: return new ChatReconfigure;
        case 39: return new PlaqueVisited;
        case 40: return new CombineRequest;
        case 56: return new EntityInfoRequest;
        case 62: return new LocationVisited;
        case 64: return new SwitchViewPoint;
        case 65: return new SaveClientOptions;
        case 67: return new DescriptionAndBattleCry;
    }
    qCWarning(logMapEvents, "Unhandled command event type %d", opcode);
    return nullptr;
}

//! @}
