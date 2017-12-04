#pragma once
#include "CRUD_Events.h"

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
    EVENT_DECL(evChatDividerMoved       ,36)
    EVENT_DECL(evPreUpdateCommand       ,1013)  // this wraps a list of commands
// server -> client commands
    EVENT_DECL(evFirstServerToClient    ,100)
    EVENT_DECL(evStandardDialogCmd      ,114)
    EVENT_DECL(evInfoMessageCmd         ,115)
    EVENT_DECL(evChatMessage            ,120)
    EVENT_DECL(evFloatingDamage         ,121)
    EVENT_DECL(evSetTarget              ,132) // Click on another entity
// client -> server commands
    EVENT_DECL(evConsoleCommand         ,200)
    EVENT_DECL(evMiniMapState           ,201)
    EVENT_DECL(evClientResumedRendering ,204)
    EVENT_DECL(evCookieRequest          ,206)
    EVENT_DECL(evEnterDoor              ,209)
    EVENT_DECL(evSetDestination         ,211)
    EVENT_DECL(evWindowState            ,214)
    EVENT_DECL(evInspirationDockMode    ,217)
    EVENT_DECL(evAbortQueuedPower       ,233) // this might actually mean something different
    EVENT_DECL(evChangeStance           ,236)
    EVENT_DECL(evChangeChatType         ,237) // Change chat type using the letters in the chat window 'l, b, g, sg, r, f"
    EVENT_DECL(evChatReconfigure        ,238)
    EVENT_DECL(evPlaqueVisited          ,239)
    EVENT_DECL(evCombineRequest         ,240)
    EVENT_DECL(evEntityInfoRequest      ,256)
    EVENT_DECL(evLocationVisited        ,262)
    EVENT_DECL(evSwitchViewPoint        ,264)
    EVENT_DECL(evClientSettings         ,265)
    EVENT_DECL(evDescriptionAndBattleCry,267)
    END_EVENTS(500)
};
