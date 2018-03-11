/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

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
typedef CRUDLink_Event MapLinkEvent; //<MapLink>

//////////////////////////////////////////////////////////////////////////
// Client -> Server
#include "Events/NewEntity.h"

class MapUnknownRequest final : public MapLinkEvent
{
public:
    MapUnknownRequest():MapLinkEvent(MapEventTypes::evUnknownEvent)
    {
    }
    void serializeto(BitStream &) const override
    {
    }
    void serializefrom(BitStream &) override
    {
    }
};
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
};

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
};

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
};
class ClientQuit final : public MapLinkEvent
{
public:
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

};
class ForcedLogout final : public MapLinkEvent
{
public:
    QString reason;
    ForcedLogout(const QString &_reason) :MapLinkEvent(MapEventTypes::evForceLogout),reason(_reason)
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
};

class CookieRequest final : public MapLinkEvent
{
public:
    uint32_t cookie;
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
};
class ConsoleCommand final : public MapLinkEvent
{
public:
    QString contents;
    ConsoleCommand():MapLinkEvent(MapEventTypes::evConsoleCommand)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,0); // opcode - Warning -> exactly the same as Connect
    }
    void serializefrom(BitStream &bs)
    {
        bs.GetString(contents);
    }
};
class ClientResumedRendering final : public MapLinkEvent
{
public:
    ClientResumedRendering():MapLinkEvent(MapEventTypes::evClientResumedRendering)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,4);
    }
    void serializefrom(BitStream &bs)
    {
    }
};

class MiniMapState final : public MapLinkEvent
{

public:
    uint32_t tile_idx=0;
    MiniMapState():MapLinkEvent(MapEventTypes::evMiniMapState)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,1); // opcode - Warning -> exactly the same as Connect
    }
    void serializefrom(BitStream &bs)
    {
        tile_idx=bs.GetPackedBits(8);
    }

};

class CombineRequest final : public MapLinkEvent
{
public:
    struct PowerEntry
    {
        int powerset_array_index;
        int powerset_index;
        int index;
    };
    PowerEntry first_power;
    PowerEntry second_power;
    CombineRequest() : MapLinkEvent(MapEventTypes::evCombineRequest)
    {}
    void serializeto(BitStream &bs) const
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
    void serializefrom(BitStream &bs)
    {
        getPowerForCombinde(bs,first_power);
        getPowerForCombinde(bs,second_power);
    }
};
#include "Events/InputState.h"
#include "Events/ChatMessage.h"
#include "Events/WindowState.h"
//////////////////////////////////////////////////////////////////////////
// Server -> Client events
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

    void        serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,5); //opcode
        bs.StorePackedBits(1,m_resp);
        if(m_resp)
            bs.StoreString(m_fatal_error);
    }
    void        serializefrom(BitStream &src)
    {
        m_resp = src.GetPackedBits(1);
        if(m_resp==0)
            src.GetString(m_fatal_error);
    }
    uint32_t    m_resp;
    QString     m_fatal_error;

};


class InspirationDockMode final : public MapLinkEvent
{
public:
    uint32_t dock_mode = 0;
    InspirationDockMode():MapLinkEvent(MapEventTypes::evInspirationDockMode)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,17);
        bs.StoreBits(32,dock_mode);
    }
    void serializefrom(BitStream &bs)
    {
        dock_mode = bs.GetBits(32);
    }
};

class EnterDoor final : public MapLinkEvent
{
public:
    bool unspecified_location;
    glm::vec3 location;
    QString name;
    EnterDoor():MapLinkEvent(MapEventTypes::evEnterDoor)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,9);
    }
    void serializefrom(BitStream &bs)
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
};
class ChangeStance final : public MapLinkEvent
{
public:
    bool enter_stance;
    int powerset_index;
    int power_index;
    ChangeStance():MapLinkEvent(MapEventTypes::evChangeStance)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,36);
    }
    void serializefrom(BitStream &bs)
    {
        enter_stance = bs.GetBits(1);
        if(!enter_stance)
            return;
        powerset_index=bs.GetPackedBits(4);
        power_index=bs.GetPackedBits(4);
    }

};
class SetDestination final : public MapLinkEvent
{
public:
    glm::vec3 destination;
    int point_index;
    SetDestination():MapLinkEvent(MapEventTypes::evSetDestination)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,11);
        bs.StoreFloat(destination.x);
        bs.StoreFloat(destination.y);
        bs.StoreFloat(destination.z);
        bs.StorePackedBits(1,point_index);
    }
    void serializefrom(BitStream &bs)
    {
        destination.x = bs.GetFloat();
        destination.y = bs.GetFloat();
        destination.z = bs.GetFloat();
        point_index   = bs.GetPackedBits(1);
    }
};

class ActivateInspiration final : public MapLinkEvent
{
public:
    int insp_unk;
    int slot_idx;
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
        insp_unk = bs.GetBits(29);
        slot_idx = bs.GetPackedBits(3);
        row_idx = bs.GetPackedBits(3);
    }
};

class UnqueueAll final : public MapLinkEvent
{
public:
    int32_t g_input_pak; // TODO: Not sure what this is?
    UnqueueAll():MapLinkEvent(MapEventTypes::evUnqueueAll)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,32);
    }
    void serializefrom(BitStream &bs)
    {
        g_input_pak = bs.GetBits(32);
    }
};

class AbortQueuedPower final : public MapLinkEvent
{
public:
    AbortQueuedPower():MapLinkEvent(MapEventTypes::evAbortQueuedPower)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,33);
    }
    void serializefrom(BitStream &bs)
    {
    }
};
class DescriptionAndBattleCry final : public MapLinkEvent
{
public:
    QString description;
    QString battlecry;
    DescriptionAndBattleCry():MapLinkEvent(MapEventTypes::evDescriptionAndBattleCry)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,67);
    }
    void serializefrom(BitStream &bs)
    {
        bs.GetString(description);
        bs.GetString(battlecry);
    }
};
class EntityInfoRequest final : public MapLinkEvent
{
public:
    QString description = "TEST Desc Sent!";
    int entity_idx;
    EntityInfoRequest():MapLinkEvent(MapEventTypes::evEntityInfoRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(12,entity_idx);
    }
    void serializefrom(BitStream &bs) override
    {
        entity_idx = bs.GetPackedBits(12);
        // TODO: What happens here?
        // bs.StorePackedBits(1,69);
        // bs.StoreString(description);
    }
};
class ReceivePlayerInfo final : public MapLinkEvent
{
public:
    QString description = "TEST Desc Sent!";
    ReceivePlayerInfo():MapLinkEvent(MapEventTypes::evReceivePlayerInfo)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,69);
        bs.StoreString(description);
    }
    void serializefrom(BitStream &bs) override
    {
        bs.GetString(description);
    }
};
class SwitchViewPoint final : public MapLinkEvent
{
public:
    bool new_viewpoint_is_firstperson;
    SwitchViewPoint():MapLinkEvent(MapEventTypes::evSwitchViewPoint)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,64);
    }
    void serializefrom(BitStream &bs)
    {
        new_viewpoint_is_firstperson = bs.GetBits(1);
    }
};
class TargetChatChannelSelected final : public MapLinkEvent
{
public:
    uint8_t m_chat_type;
    TargetChatChannelSelected():MapLinkEvent(MapEventTypes::evTargetChatChannelSelected)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,37);
    }
    void serializefrom(BitStream &bs)
    {
        m_chat_type = bs.GetPackedBits(1);
    }
};
class ChatReconfigure final : public MapLinkEvent
{
public:
    int m_chat_top_flags;
    int m_chat_bottom_flags;
    ChatReconfigure():MapLinkEvent(MapEventTypes::evChatReconfigure)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,38);
    }
    void serializefrom(BitStream &bs)
    {
        m_chat_top_flags = bs.GetPackedBits(1);
        m_chat_bottom_flags = bs.GetPackedBits(1);
    }
};
class PowersDockMode final : public MapLinkEvent
{
public:
    uint32_t dock_mode = 0;
    bool toggle_secondary_tray = 0;

    PowersDockMode():MapLinkEvent(MapEventTypes::evPowersDockMode)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,18);
        bs.StoreBits(1,toggle_secondary_tray);
    }
    void serializefrom(BitStream &bs)
    {
        dock_mode = bs.GetPackedBits(32); // Some kind of array of slotted powers?
        toggle_secondary_tray = bs.GetBits(1);
    }
};
class SwitchTray final : public MapLinkEvent
{
public:
    uint32_t tray1_num = 0;
    uint32_t tray2_num = 0;
    uint32_t tray_unk1 = 0;
    SwitchTray():MapLinkEvent(MapEventTypes::evSwitchTray)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,8);
        bs.StorePackedBits(32,tray1_num);
        bs.StorePackedBits(32,tray2_num);
        bs.StoreBits(1,tray_unk1);
    }
    void serializefrom(BitStream &bs)
    {
        tray1_num = bs.GetPackedBits(32); // Appears to correlate to Tray1's #
        tray2_num = bs.GetPackedBits(32); // Appears to correlate to Tray2's #
        tray_unk1 = bs.GetBits(1);        // TODO: Unused bits!?
        // TODO: "Console command received " blank 40 times?
    }
};

#include "Events/SaveClientOptions.h"
#include "Events/GameCommandList.h"
#include "Events/ChatDividerMoved.h"
#include "Events/SceneEvent.h"
#include "Events/EntitiesResponse.h"
#include "Events/Shortcuts.h"
#include "Events/PlaqueVisited.h"
#include "Events/LocationVisited.h"
