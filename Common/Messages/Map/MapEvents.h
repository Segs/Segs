/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Messages/Map/MapEventTypes.h"
#include "GameData/Powers.h"
#include "Components/LinkLevelEvent.h"
#include "Components/BitStream.h"
//#include "CRUD_Link.h"
//#include "CRUD_Events.h"
//#include "MapLink.h"

#include <glm/vec3.hpp>
#include <QtCore/QString>
#include <QtCore/QDebug>

class Entity;

//////////////////////////////////////////////////////////////////////////
// Client -> Server
#include "Messages/Map/NewEntity.h"
namespace SEGSEvents
{

// [[ev_def:type]]
class InitiateMapXfer final : public MapLinkEvent
{
public:
    InitiateMapXfer() : MapLinkEvent(MapEventTypes::evInitiateMapXfer)
    {
    }
    void serializeto(BitStream &) const override
    {}

    void serializefrom(BitStream &) override
    {}
    EVENT_IMPL(InitiateMapXfer)
};

// [[ev_def:type]]
class MapXferComplete final : public MapLinkEvent
{
public:
    MapXferComplete() : MapLinkEvent(MapEventTypes::evMapXferComplete)
    {}

    void serializeto(BitStream &) const override
    {}

    void serializefrom(BitStream &) override
    {}
    EVENT_IMPL(MapXferComplete)
};

// [[ev_def:type]]
class ShortcutsRequest final : public MapLinkEvent
{
public:
    ShortcutsRequest():MapLinkEvent(MapEventTypes::evShortcutsRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,4); // opcode
    }
    void serializefrom(BitStream &) override
    {
    }
    EVENT_IMPL(ShortcutsRequest)
};

// [[ev_def:type]]
class SceneRequest final : public MapLinkEvent
{
public:
    SceneRequest():MapLinkEvent(MapEventTypes::evSceneRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,3); // opcode
    }
    void serializefrom(BitStream &) override
    {
    }
    EVENT_IMPL(SceneRequest)
};

// [[ev_def:type]]
class EntitiesRequest final : public MapLinkEvent
{
public:
    EntitiesRequest():MapLinkEvent(MapEventTypes::evEntitiesRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,5); // opcode
    }
    void serializefrom(BitStream &) override
    {
    }
    EVENT_IMPL(EntitiesRequest)
};

// [[ev_def:type]]
class ClientQuit final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int abort_disconnect = 0;
    ClientQuit():MapLinkEvent(MapEventTypes::evClientQuit)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,7); // opcode
        bs.StorePackedBits(1, abort_disconnect);
    }
    void serializefrom(BitStream &bs) override
    {
        abort_disconnect = bs.GetPackedBits(1);
        // TODO: try to differentiate between quit/logout ?
    }
    EVENT_IMPL(ClientQuit)

};

// [[ev_def:type]]
class CookieRequest final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t cookie;
    // [[ev_def:field]]
    uint32_t console;
    CookieRequest():MapLinkEvent(MapEventTypes::evCookieRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,6); // opcode
    }
    void serializefrom(BitStream &bs) override
    {
        cookie = bs.GetPackedBits(1);
        console = bs.GetPackedBits(1);
    }
    EVENT_IMPL(CookieRequest)
};

// [[ev_def:type]]
class ConsoleCommand final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString contents;
    ConsoleCommand():MapLinkEvent(MapEventTypes::evConsoleCommand)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,0); // opcode - Warning -> exactly the same as Connect
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(contents);
    }
    EVENT_IMPL(ConsoleCommand)
};

// [[ev_def:type]]
class ClientResumedRendering final : public MapLinkEvent
{
public:
    ClientResumedRendering():MapLinkEvent(MapEventTypes::evClientResumedRendering)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,4);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
//        Parameterless - serializefrom is no-op
    }
    EVENT_IMPL(ClientResumedRendering)
};

// [[ev_def:type]]
class MiniMapState final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t tile_idx=0;
    MiniMapState():MapLinkEvent(MapEventTypes::evMiniMapState)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,1); // opcode - Warning -> exactly the same as Connect
    }
    void serializefrom(BitStream &bs) override
    {
        tile_idx=bs.GetPackedBits(8);
    }
    EVENT_IMPL(MiniMapState)

};
} // end of SEGSEvents namespace

#include "Messages/Map/InputState.h"
#include "Messages/Map/ChatMessage.h"
#include "Messages/Map/WindowState.h"
//////////////////////////////////////////////////////////////////////////
// Server -> Client events
namespace SEGSEvents
{
// [[ev_def:type]]
class MapInstanceConnected final : public MapLinkEvent
{
public:
    MapInstanceConnected():MapLinkEvent(MapEventTypes::evMapInstanceConnected)
    {}
    MapInstanceConnected(EventProcessor *evsrc,uint32_t resp,const QString &err) :
        MapLinkEvent(MapEventTypes::evMapInstanceConnected,evsrc),
        m_resp(resp),
        m_fatal_error(err)
    {}

    void        serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,5); //opcode
        bs.StorePackedBits(1,m_resp);
        if(m_resp)
            bs.StoreString(m_fatal_error);
    }
    void        serializefrom(BitStream &src) override
    {
        m_resp = src.GetPackedBits(1);
        if(m_resp==0)
            src.GetString(m_fatal_error);
    }
    // [[ev_def:field]]
    uint32_t    m_resp;
    // [[ev_def:field]]
    QString     m_fatal_error;
    EVENT_IMPL(MapInstanceConnected)

};

// [[ev_def:type]]
class InspirationDockMode final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t dock_mode = 0;
    InspirationDockMode():MapLinkEvent(MapEventTypes::evInspirationDockMode)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,17);
        bs.StoreBits(32,dock_mode);
    }
    void serializefrom(BitStream &bs) override
    {
        dock_mode = bs.GetBits(32);
    }
    EVENT_IMPL(InspirationDockMode)
};

// [[ev_def:type]]
class DialogButton final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t button_id;
    // [[ev_def:field]]
    bool success;

    DialogButton():MapLinkEvent(MapEventTypes::evDialogButton)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,24);
    }
    void serializefrom(BitStream &bs) override
    {
        button_id = bs.GetPackedBits(1);
        if(bs.GetReadableBits() == 1)
            success = bs.GetBits(1);
    }

    EVENT_IMPL(DialogButton)
};

// [[ev_def:type]]
class UnqueueAll final : public MapLinkEvent
{
public:
    UnqueueAll():MapLinkEvent(MapEventTypes::evUnqueueAll)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,32);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
        // Seems to be properly handled elsewheres
    }
    EVENT_IMPL(UnqueueAll)
};

// [[ev_def:type]]
class AbortQueuedPower final : public MapLinkEvent
{
public:
    AbortQueuedPower():MapLinkEvent(MapEventTypes::evAbortQueuedPower)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,33);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
//        Parameterless - serializefrom is no-op
    }
    EVENT_IMPL(AbortQueuedPower)
};

// [[ev_def:type]]
class RecvSelectedTitles : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    bool m_has_prefix;
    // [[ev_def:field]]
    uint32_t m_generic;
    // [[ev_def:field]]
    uint32_t m_origin;

    RecvSelectedTitles():MapLinkEvent(MapEventTypes::evRecvSelectedTitles)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,66);
    }
    void serializefrom(BitStream &bs) override
    {
        m_has_prefix = bs.GetBits(1);
        m_generic = bs.GetPackedBits(5);
        m_origin = bs.GetPackedBits(5);
    }
    EVENT_IMPL(RecvSelectedTitles)

};

// [[ev_def:type]]
class DescriptionAndBattleCry final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString description;
    // [[ev_def:field]]
    QString battlecry;
    DescriptionAndBattleCry():MapLinkEvent(MapEventTypes::evDescriptionAndBattleCry)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,67);
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(description);
        bs.GetString(battlecry);
    }
    EVENT_IMPL(DescriptionAndBattleCry)
};

// [[ev_def:type]]
class SwitchViewPoint final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    bool new_viewpoint_is_firstperson;
    SwitchViewPoint():MapLinkEvent(MapEventTypes::evSwitchViewPoint)
    {}
    void serializeto(BitStream &bs) const  override
    {
        bs.StorePackedBits(1,64);
    }
    void serializefrom(BitStream &bs) override
    {
        new_viewpoint_is_firstperson = bs.GetBits(1);
    }
    EVENT_IMPL(SwitchViewPoint)
};

// [[ev_def:type]]
class TargetChatChannelSelected final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint8_t m_chat_type;
    TargetChatChannelSelected():MapLinkEvent(MapEventTypes::evTargetChatChannelSelected)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,37);
    }
    void serializefrom(BitStream &bs) override
    {
        m_chat_type = bs.GetPackedBits(1);
    }
    EVENT_IMPL(TargetChatChannelSelected)
};

// [[ev_def:type]]
class ChatReconfigure final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int m_chat_top_flags;
    // [[ev_def:field]]
    int m_chat_bottom_flags;
    ChatReconfigure():MapLinkEvent(MapEventTypes::evChatReconfigure)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,38);
    }
    void serializefrom(BitStream &bs) override
    {
        m_chat_top_flags = bs.GetPackedBits(1);
        m_chat_bottom_flags = bs.GetPackedBits(1);
    }
    EVENT_IMPL(ChatReconfigure)
};

// [[ev_def:type]]
class PowersDockMode final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    bool toggle_secondary_tray = 0;

    PowersDockMode():MapLinkEvent(MapEventTypes::evPowersDockMode)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,18);
        bs.StoreBits(1,toggle_secondary_tray);
    }
    void serializefrom(BitStream &bs) override
    {
        toggle_secondary_tray = bs.GetBits(1);
    }
    EVENT_IMPL(PowersDockMode)
};

// [[ev_def:type]]
class SwitchTray final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    PowerTrayGroup tray_group;

    SwitchTray():MapLinkEvent(MapEventTypes::evSwitchTray)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,8);
        //bs.StorePackedBits(32,tray1_num);
        //bs.StorePackedBits(32,tray2_num);
        tray_group.serializeto(bs);
    }
    void serializefrom(BitStream &bs) override
    {
        //tray1_num = bs.GetPackedBits(32); // Appears to correlate to Tray1's #
        //tray2_num = bs.GetPackedBits(32); // Appears to correlate to Tray2's #
        tray_group.serializefrom(bs);
    }
    EVENT_IMPL(SwitchTray)
};

// [[ev_def:type]]
class SetKeybind final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString profile;
    // [[ev_def:field]]
    uint32_t key_and_secondary;
    // [[ev_def:field]]
    uint8_t key;
    // [[ev_def:field]]
    uint32_t mods;
    // [[ev_def:field]]
    QString command;
    // [[ev_def:field]]
    bool is_secondary;

    SetKeybind():MapLinkEvent(MapEventTypes::evSetKeybind)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,19);
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(profile);  // Profile Name
        key_and_secondary = bs.GetBits(32); // Key & Secondary Binding

        key = key_and_secondary &0xFF;
        is_secondary = (key_and_secondary & 0xF00)==0xF00;

        mods = bs.GetBits(32);  // Mods
        bs.GetString(command);  // Command
    }
    EVENT_IMPL(SetKeybind)
};

// [[ev_def:type]]
class RemoveKeybind final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString profile;
    // [[ev_def:field]]
    uint32_t key;
    // [[ev_def:field]]
    uint32_t mods;
    RemoveKeybind():MapLinkEvent(MapEventTypes::evRemoveKeybind)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,20);
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(profile);  // Profile Name
        key = bs.GetBits(32);   // Key
        mods = bs.GetBits(32);  // Mods
    }
    EVENT_IMPL(RemoveKeybind)
};

// [[ev_def:type]]
class ResetKeybinds final : public MapLinkEvent
{
public:
    ResetKeybinds():MapLinkEvent(MapEventTypes::evResetKeybinds)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,21);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
        // TODO: Seems like nothing is received server side.
    }
    EVENT_IMPL(ResetKeybinds)
};

// [[ev_def:type]]
class SelectKeybindProfile final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString profile;
    SelectKeybindProfile():MapLinkEvent(MapEventTypes::evSelectKeybindProfile)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,22);
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(profile); // Keybind Profile Name
    }
    EVENT_IMPL(SelectKeybindProfile)
};

// [[ev_def:type]]
class MoveInspiration final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t src_col;
    // [[ev_def:field]]
    uint32_t src_row;
    // [[ev_def:field]]
    uint32_t dest_col;
    // [[ev_def:field]]
    uint32_t dest_row;
    MoveInspiration():MapLinkEvent(MapEventTypes::evMoveInspiration)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,34);
    }
    void serializefrom(BitStream &bs) override
    {
        src_col = bs.GetPackedBits(3);
        src_row = bs.GetPackedBits(3);
        dest_col = bs.GetPackedBits(3);
        dest_row = bs.GetPackedBits(3);
    }
    EVENT_IMPL(MoveInspiration)

};
} // end of SEGSEvents namespace

#include "Messages/Map/Browser.h"
#include "Messages/Map/ChatDividerMoved.h"
#include "Messages/Map/ContactDialogs.h"
#include "Messages/Map/DeadNoGurney.h"
#include "Messages/Map/DoorAnims.h"
#include "Messages/Map/EntitiesResponse.h"
#include "Messages/Map/FriendsListUpdate.h"
#include "Messages/Map/InteractWithEntity.h"
#include "Messages/Map/LocationVisited.h"
#include "Messages/Map/PlaqueVisited.h"
#include "Messages/Map/PowerSystemEvents.h"
#include "Messages/Map/SaveClientOptions.h"
#include "Messages/Map/SceneEvent.h"
#include "Messages/Map/SendStance.h"
#include "Messages/Map/Shortcuts.h"
#include "Messages/Map/TailorOpen.h"
#include "Messages/Map/TradeCancel.h"
#include "Messages/Map/TradeInit.h"
#include "Messages/Map/TradeOffer.h"
#include "Messages/Map/TradeSuccess.h"
#include "Messages/Map/TradeUpdate.h"
#include "Messages/Map/TradeWasCancelledMessage.h"
#include "Messages/Map/TradeWasUpdatedMessage.h"
#include "Messages/Map/Waypoints.h"
