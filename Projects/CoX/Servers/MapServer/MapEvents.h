/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Events/MapEventTypes.h"
#include "LinkLevelEvent.h"
#include "BitStream.h"
#include "CRUD_Link.h"
#include "CRUD_Events.h"
#include "MapLink.h"

#include <glm/vec3.hpp>
#include <QtCore/QString>
#include <QtCore/QDebug>

class Entity;
namespace SEGSEvents
{
using MapLinkEvent = CRUDLink_Event; //<MapLink>
}

//////////////////////////////////////////////////////////////////////////
// Client -> Server
#include "Events/NewEntity.h"

namespace SEGSEvents
{

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
class ForcedLogout final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    QString reason;
    ForcedLogout() :MapLinkEvent(MapEventTypes::evForcedLogout) {}
    ForcedLogout(const QString &_reason) :MapLinkEvent(MapEventTypes::evForcedLogout),reason(_reason)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,12); // opcode
        bs.StoreString(reason); // opcode
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(reason);
    }
    EVENT_IMPL(ForcedLogout)
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

// [[ev_def:type]]
class CombineRequest final : public MapLinkEvent
{
public:
    struct PowerEntry
    {
        int powerset_array_index;
        int powerset_index;
        int index;
        template <class Archive>
        void serialize( Archive & ar )
        {
            ar( powerset_array_index,powerset_index,index );
        }
    };
    // [[ev_def:field]]
    PowerEntry first_power;
    // [[ev_def:field]]
    PowerEntry second_power;
    CombineRequest() : MapLinkEvent(MapEventTypes::evCombineRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,40); // opcode
        assert(false); // since we will not send CombineRequest to anyone :)
    }
    // now to make this useful ;)

    void getPowerForCombinde(BitStream &bs,PowerEntry &entry)
    {
        // first bit tells us if we have full/partial?? data
        // here we can do a small refactoring, because in both branches of if/else, the last set value is index.
        if(bs.GetBits(1))
        {
            entry.powerset_array_index = bs.GetPackedBits(1);
            entry.powerset_index = bs.GetPackedBits(1);
        }
        entry.index = bs.GetPackedBits(1);
    }
    void serializefrom(BitStream &bs) override
    {
        getPowerForCombinde(bs,first_power);
        getPowerForCombinde(bs,second_power);
    }
    EVENT_IMPL(CombineRequest)
};
} // end of SEGSEvents namespace

#include "Events/InputState.h"
#include "Events/ChatMessage.h"
#include "Events/WindowState.h"
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
class EnterDoor final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    bool unspecified_location;
    // [[ev_def:field]]
    glm::vec3 location;
    // [[ev_def:field]]
    QString name;
    EnterDoor():MapLinkEvent(MapEventTypes::evEnterDoor)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,9);
    }
    void serializefrom(BitStream &bs) override
    {
        unspecified_location = bs.GetBits(1);
        if(!unspecified_location)
        {
            location.x = bs.GetFloat();
            location.y = bs.GetFloat();
            location.z = bs.GetFloat();
        }
        bs.GetString(name);
    }
    EVENT_IMPL(EnterDoor)
};

// [[ev_def:type]]
class ChangeStance final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    bool enter_stance;
    // [[ev_def:field]]
    int powerset_index;
    // [[ev_def:field]]
    int power_index;
    ChangeStance():MapLinkEvent(MapEventTypes::evChangeStance)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,36);
    }
    void serializefrom(BitStream &bs) override
    {
        enter_stance = bs.GetBits(1);
        if(!enter_stance)
            return;
        powerset_index=bs.GetPackedBits(4);
        power_index=bs.GetPackedBits(4);
    }
    EVENT_IMPL(ChangeStance)

};

// [[ev_def:type]]
class SetDestination final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    glm::vec3 destination;
    // [[ev_def:field]]
    int point_index;
    SetDestination():MapLinkEvent(MapEventTypes::evSetDestination)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,11);
        bs.StoreFloat(destination.x);
        bs.StoreFloat(destination.y);
        bs.StoreFloat(destination.z);
        bs.StorePackedBits(1,point_index);
    }
    void serializefrom(BitStream &bs) override
    {
        destination.x = bs.GetFloat();
        destination.y = bs.GetFloat();
        destination.z = bs.GetFloat();
        point_index   = bs.GetPackedBits(1);
    }
    EVENT_IMPL(SetDestination)
};

// [[ev_def:type]]
class ActivateInspiration final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int slot_idx;
    // [[ev_def:field]]
    int row_idx;
    ActivateInspiration():MapLinkEvent(MapEventTypes::evActivateInspiration)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,29);
        bs.StorePackedBits(3,slot_idx);
        bs.StorePackedBits(3,row_idx);
    }
    void serializefrom(BitStream &bs) override
    {
        slot_idx = bs.GetPackedBits(3);
        row_idx = bs.GetPackedBits(3);
    }
    EVENT_IMPL(ActivateInspiration)
};

// [[ev_def:type]]
class SetDefaultPowerSend final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int powerset_idx;
    // [[ev_def:field]]
    int power_idx;
    SetDefaultPowerSend():MapLinkEvent(MapEventTypes::evSetDefaultPowerSend)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,30);
    }
    void serializefrom(BitStream &bs) override
    {
        powerset_idx = bs.GetPackedBits(4);
        power_idx = bs.GetPackedBits(4);
    }
    EVENT_IMPL(SetDefaultPowerSend)
};

// [[ev_def:type]]
class SetDefaultPower final : public MapLinkEvent
{
public:
    SetDefaultPower():MapLinkEvent(MapEventTypes::evSetDefaultPower)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,31);
    }
    void serializefrom(BitStream &/*bs*/) override
    {
        // TODO: Seems like nothing is received server side.
        qWarning() << "SetDefaultPower unimplemented.";
    }
    EVENT_IMPL(SetDefaultPower)
};

// [[ev_def:type]]
class UnqueueAll final : public MapLinkEvent
{
public:
    UnqueueAll():MapLinkEvent(MapEventTypes::evUnqueueAll)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,32);
    }
    void serializefrom(BitStream &/*bs*/)
    {
        // TODO: Seems like nothing is received server side.
        qWarning() << "UnqueueAll unimplemented.";
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
    uint32_t tray1_num = 0;
    // [[ev_def:field]]
    uint32_t tray2_num = 0;
    // [[ev_def:field]]
    uint32_t tray_unk1 = 0;
    SwitchTray():MapLinkEvent(MapEventTypes::evSwitchTray)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,8);
        bs.StorePackedBits(32,tray1_num);
        bs.StorePackedBits(32,tray2_num);
        bs.StoreBits(1,tray_unk1);
    }
    void serializefrom(BitStream &bs) override
    {
        tray1_num = bs.GetPackedBits(32); // Appears to correlate to Tray1's #
        tray2_num = bs.GetPackedBits(32); // Appears to correlate to Tray2's #
        tray_unk1 = bs.GetBits(1);        // TODO: Unused bits!?
        // TODO: "Console command received " blank 40 times?
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
    EVENT_IMPL(ResetKeybind)
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
} // end of SEGSEvents namespace

#include "Events/ChatDividerMoved.h"
#include "Events/EntitiesResponse.h"
#include "Events/FriendsListUpdate.h"
#include "Events/GameCommandList.h"
#include "Events/LocationVisited.h"
#include "Events/PlaqueVisited.h"
#include "Events/PlayerInfo.h"
#include "Events/SaveClientOptions.h"
#include "Events/SceneEvent.h"
#include "Events/Shortcuts.h"
#include "Events/SidekickOffer.h"
#include "Events/TeamLooking.h"
#include "Events/TeamOffer.h"
#include "Events/InteractWithEntity.h"
