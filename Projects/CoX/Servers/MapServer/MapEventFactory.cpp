/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapEventFactory.h"
#include "Messages/Map/MapEvents.h"
#include "Common/Messages/Map/ClueList.h"
#include "Common/Messages/Map/ContactList.h"
#include "Common/Messages/Map/LevelUp.h"
#include "Common/Messages/Map/PlayerInfo.h"
#include "Common/Messages/Map/StoresEvents.h"
#include "Common/Messages/Map/Tasks.h"

using namespace SEGSEvents;

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
        case 2: return new RecvInputState;
        case 3: return new SceneRequest;
        case 4: return new ShortcutsRequest;
        case 5: return new EntitiesRequest;
        case 6: return new CookieRequest;
        case 7: return new ClientQuit;
        case 9: return new NewEntity;
        case 11: return new InitiateMapXfer;
        case 12: return new MapXferComplete;
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
            return new Idle;
        case 1: return new MiniMapState;
        case 4: return new ClientResumedRendering;
        case 7: return new InteractWithEntity;
        case 8: return new SwitchTray;
        case 9: return new EnterDoor;
        case 10: return new AwaitingDeadNoGurney;
        case 11: return new SetDestination;
        case 13: return new HasEnteredDoor;
        case 14: return new WindowState;
        case 16: return new ChatDividerMoved;
        case 17: return new InspirationDockMode;
        case 18: return new PowersDockMode;
        case 19: return new SetKeybind;
        case 20: return new RemoveKeybind;
        case 21: return new ResetKeybinds;
        case 22: return new SelectKeybindProfile;
        case 24: return new DialogButton;
        case 26: return new ReceiveContactStatus;
        case 27: return new ActivatePower;
        case 28: return new ActivatePowerAtLocation;
        case 29: return new ActivateInspiration;
        case 30: return new SetDefaultPower;
        case 31: return new UnsetDefaultPower;
        case 32: return new UnqueueAll;
        case 33: return new AbortQueuedPower;
        case 34: return new MoveInspiration;
        case 35: return new DeadNoGurneyOK;
        case 36: return new ChangeStance;
        case 37: return new TargetChatChannelSelected;
        case 38: return new ChatReconfigure;
        case 39: return new PlaqueVisited;
        case 40: return new CombineEnhancementsReq;
        case 41: return new MoveEnhancement;
        case 42: return new SetEnhancement;
        case 43: return new TrashEnhancement;
        case 44: return new TrashEnhancementInPower;
        case 45: return new BuyEnhancementSlot;
        case 46: return new RecvNewPower;
        case 47: return new LevelUpResponse;
        case 54: return new TradeWasCancelledMessage;
        case 55: return new TradeWasUpdatedMessage;
        case 56: return new EntityInfoRequest;
        case 60: return new RecvCostumeChange;
        case 62: return new LocationVisited;
        case 63: return new ReceiveTaskDetailRequest;
        case 64: return new SwitchViewPoint;
        case 65: return new SaveClientOptions;
        case 66: return new RecvSelectedTitles;
        case 67: return new DescriptionAndBattleCry;
        case 68: return new SouvenirDetailRequest;
        case 69: return new StoreBuyItem;
        case 70: return new StoreSellItem;
        case 77: return new BrowserClose;
    }
    qCWarning(logMapEvents, "Unhandled command event type %d", opcode);
    return nullptr;
}

//! @}
