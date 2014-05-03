/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once
#include <ace/Assert.h>
#include "LinkLevelEvent.h"
#include "BitStream.h"
#include "CRUD_Link.h"
#include "CRUD_Events.h"
#include "MapLink.h"
class Entity;
typedef CRUDLink_Event MapLinkEvent; //<MapLink>

class MapEventTypes : public CRUD_EventTypes
{
public:
    BEGINE_EVENTS(CRUD_EventTypes)
    EVENT_DECL(evEntityEnteringMap      ,0)
    EVENT_DECL(evMapInstanceConnected   ,1)
    EVENT_DECL(evShortcutsRequest       ,2)
    EVENT_DECL(evShortcuts              ,3)
    EVENT_DECL(evSceneRequest           ,4)
    EVENT_DECL(evScene                  ,5)
    EVENT_DECL(evEntitiesRequest        ,6)
    EVENT_DECL(evEntitites              ,7)
    EVENT_DECL(evInputState             ,8)
    EVENT_DECL(evClientQuit             ,9)
    EVENT_DECL(evForceLogout            ,10)
    EVENT_DECL(evChatMessage            ,20)
    EVENT_DECL(evCombineRequest         ,40)
    EVENT_DECL(evConsoleCommand         ,100)
    EVENT_DECL(evMiniMapState           ,101)
    EVENT_DECL(evCookieRequest          ,106)
    EVENT_DECL(evWindowState            ,114)
    END_EVENTS(500)
};
//////////////////////////////////////////////////////////////////////////
// Client -> Server
#include "Events/NewEntity.h"

class MapUnknownRequest : public MapLinkEvent
{
public:
    MapUnknownRequest():MapLinkEvent(MapEventTypes::evUnknownEvent)
    {

    }
    void serializeto(BitStream &) const
    {
    }
    void serializefrom(BitStream &)
    {
    }
};
class ShortcutsRequest : public MapLinkEvent
{
public:
    ShortcutsRequest():MapLinkEvent(MapEventTypes::evShortcutsRequest)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,4); // opcode
    }
    void serializefrom(BitStream &)
    {
    }
};

class SceneRequest : public MapLinkEvent
{
public:
    SceneRequest():MapLinkEvent(MapEventTypes::evSceneRequest)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,3); // opcode
    }
    void serializefrom(BitStream &)
    {
    }
};

class EntitiesRequest : public MapLinkEvent
{
public:
    EntitiesRequest():MapLinkEvent(MapEventTypes::evEntitiesRequest)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,5); // opcode
    }
    void serializefrom(BitStream &)
    {
    }
};
class ClientQuit : public MapLinkEvent {
public:
    int reason;
    ClientQuit():MapLinkEvent(MapEventTypes::evClientQuit)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,7); // opcode
    }
    void serializefrom(BitStream &bs)
    {
        reason = bs.GetPackedBits(1);
        // TODO: try to differentiate between quit/logout ?
    }

};
class ForcedLogout: public MapLinkEvent {
public:
    std::string reason;
    ForcedLogout(const std::string &_reason) :MapLinkEvent(MapEventTypes::evForceLogout),reason(_reason) {

    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,12); // opcode
        bs.StoreString(reason); // opcode
    }
    void serializefrom(BitStream &bs)
    {
        bs.GetString(reason);
    }
};

class CookieRequest : public MapLinkEvent
{
public:
    uint32_t cookie;
    uint32_t console;
    CookieRequest():MapLinkEvent(MapEventTypes::evCookieRequest)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,6); // opcode
    }
    void serializefrom(BitStream &bs)
    {
        cookie = bs.GetPackedBits(1);
        console = bs.GetPackedBits(1);
    }
};
class ConsoleCommand : public MapLinkEvent {
public:
    std::string contents;
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

class MiniMapState : public MapLinkEvent {

public:
    uint32_t tile_idx;
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
class WindowState : public MapLinkEvent
{
public:
    uint32_t window_idx;
    struct WindowS {
        uint32_t field_0;
        uint32_t field_4;
        uint32_t field_8;
        uint32_t field_C;
        uint32_t field_14;
        uint32_t field_18;
        uint32_t field_24;
        uint32_t color;
        uint32_t alpha;
    };
    WindowS wnd;
    WindowState():MapLinkEvent(MapEventTypes::evWindowState)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,14); // opcode
    }
    void serializefrom(BitStream &bs)
    {
        window_idx = bs.GetPackedBits(1);

        wnd.field_0 = bs.GetPackedBits(1);
        wnd.field_4 = bs.GetPackedBits(1);
        uint32_t val = bs.GetPackedBits(1);
        if(val==4)
            wnd.field_18 = 2;
        wnd.field_24 = val;

        wnd.field_14 = bs.GetPackedBits(1);
        wnd.color = bs.GetPackedBits(1);
        wnd.alpha = bs.GetPackedBits(1);
        if(bs.GetBits(1)) {
            wnd.field_8 = bs.GetPackedBits(1);
            wnd.field_C = bs.GetPackedBits(1);
        }
    }
};

class CombineRequest : public MapLinkEvent
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
//////////////////////////////////////////////////////////////////////////
// Server -> Client events
class MapInstanceConnected : public MapLinkEvent
{
public:
    MapInstanceConnected():MapLinkEvent(MapEventTypes::evMapInstanceConnected)
    {}
    MapInstanceConnected(EventProcessor *evsrc,uint32_t resp,const std::string &err) :
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
    std::string m_fatal_error;

};

#include "Events/ChatMessage.h"
#include "Events/SceneEvent.h"
#include "Events/EntitiesResponse.h"
#include "Events/Shortcuts.h"
