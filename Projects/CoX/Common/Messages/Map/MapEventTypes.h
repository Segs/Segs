/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUDP_Protocol/CRUD_Events.h"
namespace SEGSEvents
{
using MapLinkEvent = CRUDLink_Event;

enum MapEventTypes
{
    BEGINE_EVENTS(MapEventTypes,CRUD_EventTypes)
    EVENT_DECL(MapEventTypes, evNewEntity                  ,0)
    EVENT_DECL(MapEventTypes, evMapInstanceConnected       ,1)
    EVENT_DECL(MapEventTypes, evShortcutsRequest           ,2)
    EVENT_DECL(MapEventTypes, evShortcuts                  ,3)
    EVENT_DECL(MapEventTypes, evSceneRequest               ,4)
    EVENT_DECL(MapEventTypes, evScene                      ,5) // Should be 106
    EVENT_DECL(MapEventTypes, evEntitiesRequest            ,6)
    EVENT_DECL(MapEventTypes, evEntitiesResponse           ,7)
    EVENT_DECL(MapEventTypes, evRecvInputState             ,8)
    EVENT_DECL(MapEventTypes, evClientQuit                 ,9)
    EVENT_DECL(MapEventTypes, evInitiateMapXfer            ,11)
    EVENT_DECL(MapEventTypes, evMapXferComplete            ,12)
    EVENT_DECL(MapEventTypes, evChatDividerMoved           ,36)
// server -> client commands
    EVENT_DECL(MapEventTypes, evFirstServerToClient        ,100)
    EVENT_DECL(MapEventTypes, evMapXferWait                ,110)
    EVENT_DECL(MapEventTypes, evMapXferRequest             ,111)
    EVENT_DECL(MapEventTypes, evForceLogout                ,112)
//    EVENT_DECL(MapEventTypes, evServerGameCommand        ,113) // 1013?
    EVENT_DECL(MapEventTypes, evStandardDialogCmd          ,114)
    EVENT_DECL(MapEventTypes, evInfoMessageCmd             ,115)
    EVENT_DECL(MapEventTypes, evDoorMessage                ,116)
    EVENT_DECL(MapEventTypes, evConsoleOutput              ,117)
    EVENT_DECL(MapEventTypes, evSetClientState             ,118)
    EVENT_DECL(MapEventTypes, evConsolePrint               ,119)
    EVENT_DECL(MapEventTypes, evChatMessage                ,120)
    EVENT_DECL(MapEventTypes, evFloatingDamage             ,121)
    EVENT_DECL(MapEventTypes, evVisitMapCells              ,122)
    EVENT_DECL(MapEventTypes, evSendWaypoint               ,123)
    EVENT_DECL(MapEventTypes, evTeamOffer                  ,124)
    EVENT_DECL(MapEventTypes, evTeamLooking                ,125)
//    EVENT_DECL(MapEventTypes, evTaskForceKick            ,126)
//    EVENT_DECL(MapEventTypes, evTaskForceQuit            ,127)
    EVENT_DECL(MapEventTypes, evSidekickOffer              ,128)
//    EVENT_DECL(MapEventTypes, evSuperGroupOffer          ,129)
//    EVENT_DECL(MapEventTypes, evSuperGroupResponse       ,130)
//    EVENT_DECL(MapEventTypes, evSuperGroupCostume        ,131)
    EVENT_DECL(MapEventTypes, evTradeOffer                 ,132)
    EVENT_DECL(MapEventTypes, evTradeInit                  ,133)
    EVENT_DECL(MapEventTypes, evTradeCancel                ,134)
    EVENT_DECL(MapEventTypes, evTradeUpdate                ,135)
    EVENT_DECL(MapEventTypes, evTradeSuccess               ,136)
    EVENT_DECL(MapEventTypes, evFriendsListUpdate          ,137)
//    EVENT_DECL(MapEventTypes, evControlPlayer            ,138)
    EVENT_DECL(MapEventTypes, evContactDialog              ,139)
    EVENT_DECL(MapEventTypes, evContactDialogClose         ,140)
    EVENT_DECL(MapEventTypes, evContactDialogOk            ,141)
    EVENT_DECL(MapEventTypes, evContactDialogYesNo         ,142)
    EVENT_DECL(MapEventTypes, evContactStatusList          ,143)
    EVENT_DECL(MapEventTypes, evContactSelect              ,144)
    EVENT_DECL(MapEventTypes, evTaskStatusList             ,145)
    EVENT_DECL(MapEventTypes, evTaskSelect                 ,146)
    EVENT_DECL(MapEventTypes, evTaskListRemoveTeammates    ,147)
    EVENT_DECL(MapEventTypes, evTimeUpdate                 ,148)
//    EVENT_DECL(MapEventTypes, evMissionEntryText         ,149)
//    EVENT_DECL(MapEventTypes, evMissionKick              ,150)
    EVENT_DECL(MapEventTypes, evDeadNoGurney               ,151)
    EVENT_DECL(MapEventTypes, evDoorAnimStart              ,152)
    EVENT_DECL(MapEventTypes, evDoorAnimExit               ,153)
    EVENT_DECL(MapEventTypes, evFaceEntity                 ,154)
    EVENT_DECL(MapEventTypes, evFaceLocation               ,155)
    EVENT_DECL(MapEventTypes, evFloatingInfo               ,156)
    EVENT_DECL(MapEventTypes, evSendStance                 ,157)
    EVENT_DECL(MapEventTypes, evMapXferList                ,158)
    EVENT_DECL(MapEventTypes, evMapXferListClose           ,159)
    EVENT_DECL(MapEventTypes, evAddTimeStateLog            ,160)
    EVENT_DECL(MapEventTypes, evLevelUp                    ,161)
    EVENT_DECL(MapEventTypes, evChangeTitle                ,162)
    EVENT_DECL(MapEventTypes, evSendLocations              ,163)
//    EVENT_DECL(MapEventTypes, evCSRBugReport             ,164)
    EVENT_DECL(MapEventTypes, evEmailHeaders               ,165)
    EVENT_DECL(MapEventTypes, evEmailRead                  ,166)
    EVENT_DECL(MapEventTypes, evEmailMessageStatus         ,167)
    EVENT_DECL(MapEventTypes, evEntityInfoResponse         ,169) // Send entity info (description)
    EVENT_DECL(MapEventTypes, evClueList                   ,170)
    EVENT_DECL(MapEventTypes, evSouvenirListHeaders        ,171)
    EVENT_DECL(MapEventTypes, evSouvenirDetail             ,172)
    EVENT_DECL(MapEventTypes, evMissionObjectiveTimer      ,173)
    EVENT_DECL(MapEventTypes, evTaskDetail                 ,175)
    EVENT_DECL(MapEventTypes, evBrowser                    ,176)
    EVENT_DECL(MapEventTypes, evStoreOpen                  ,178)
    EVENT_DECL(MapEventTypes, evStoreClose                 ,179)
    EVENT_DECL(MapEventTypes, evTrayAdd                    ,182)
    EVENT_DECL(MapEventTypes, evCombineEnhanceResponse     ,183)
    EVENT_DECL(MapEventTypes, evTailorOpen                 ,188)
// client -> server commands
    EVENT_DECL(MapEventTypes, evConsoleCommand             ,200)
    EVENT_DECL(MapEventTypes, evMiniMapState               ,201)
    EVENT_DECL(MapEventTypes, evClientResumedRendering     ,204)
    EVENT_DECL(MapEventTypes, evCookieRequest              ,206)
    EVENT_DECL(MapEventTypes, evInteractWithEntity         ,207)
    EVENT_DECL(MapEventTypes, evSwitchTray                 ,208) // Switch Tray using left-right arrows next to power tray
    EVENT_DECL(MapEventTypes, evEnterDoor                  ,209)
    EVENT_DECL(MapEventTypes, evAwaitingDeadNoGurney       ,210)
    EVENT_DECL(MapEventTypes, evSetDestination             ,211)
    EVENT_DECL(MapEventTypes, evHasEnteredDoor             ,213)
    EVENT_DECL(MapEventTypes, evWindowState                ,214)
    EVENT_DECL(MapEventTypes, evInspirationDockMode        ,217)
    EVENT_DECL(MapEventTypes, evPowersDockMode             ,218)
    EVENT_DECL(MapEventTypes, evSetKeybind                 ,219)
    EVENT_DECL(MapEventTypes, evRemoveKeybind              ,220)
    EVENT_DECL(MapEventTypes, evResetKeybinds              ,221)
    EVENT_DECL(MapEventTypes, evSelectKeybindProfile       ,222)
    EVENT_DECL(MapEventTypes, evDialogButton               ,224)
    EVENT_DECL(MapEventTypes, evReceiveContactStatus       ,226)
    EVENT_DECL(MapEventTypes, evActivatePower              ,227)
    EVENT_DECL(MapEventTypes, evActivatePowerAtLocation    ,228)
    EVENT_DECL(MapEventTypes, evActivateInspiration        ,229) // Hit F1, F2, F3, F4, F5 or Click on Inspiration in Tray
    EVENT_DECL(MapEventTypes, evSetDefaultPower            ,230) // Send Default Powerset
    EVENT_DECL(MapEventTypes, evUnsetDefaultPower          ,231) // Clicking 'z'
    EVENT_DECL(MapEventTypes, evUnqueueAll                 ,232) // Click on another entity or Clicking 'z'
    EVENT_DECL(MapEventTypes, evAbortQueuedPower           ,233) // Clicking 'z'
    EVENT_DECL(MapEventTypes, evMoveInspiration            ,234)
    EVENT_DECL(MapEventTypes, evDeadNoGurneyOK             ,235) // player click OK on death dialog
    EVENT_DECL(MapEventTypes, evChangeStance               ,236)
    EVENT_DECL(MapEventTypes, evTargetChatChannelSelected  ,237) // Change chat type using the letters in the chat window 'l, b, g, sg, r, f"
    EVENT_DECL(MapEventTypes, evChatReconfigure            ,238)
    EVENT_DECL(MapEventTypes, evPlaqueVisited              ,239)
    EVENT_DECL(MapEventTypes, evCombineEnhancementsReq     ,240)
    EVENT_DECL(MapEventTypes, evMoveEnhancement            ,241)
    EVENT_DECL(MapEventTypes, evSetEnhancement             ,242)
    EVENT_DECL(MapEventTypes, evTrashEnhancement           ,243)
    EVENT_DECL(MapEventTypes, evTrashEnhancementInPower    ,244)
    EVENT_DECL(MapEventTypes, evBuyEnhancementSlot         ,245)
    EVENT_DECL(MapEventTypes, evRecvNewPower               ,246)
    EVENT_DECL(MapEventTypes, evLevelUpResponse            ,247)
    EVENT_DECL(MapEventTypes, evTradeWasCancelledMessage   ,254) // Trade window: player clicked "Cancel" or closed the window.
    EVENT_DECL(MapEventTypes, evTradeWasUpdatedMessage     ,255) // Trade window: player changed something in the window.
    EVENT_DECL(MapEventTypes, evEntityInfoRequest          ,256) // right-click entity select "Info"
    EVENT_DECL(MapEventTypes, evRecvCostumeChange          ,260)
    EVENT_DECL(MapEventTypes, evLocationVisited            ,262)
    EVENT_DECL(MapEventTypes, evReceiveTaskDetailRequest   ,263)
    EVENT_DECL(MapEventTypes, evSwitchViewPoint            ,264)
    EVENT_DECL(MapEventTypes, evSaveClientOptions          ,265)
    EVENT_DECL(MapEventTypes, evRecvSelectedTitles         ,266)
    EVENT_DECL(MapEventTypes, evDescriptionAndBattleCry    ,267)
    EVENT_DECL(MapEventTypes, evSouvenirDetailRequest      ,268)
    EVENT_DECL(MapEventTypes, evStoreBuyItem               ,269)
    EVENT_DECL(MapEventTypes, evStoreSellItem              ,270)
    EVENT_DECL(MapEventTypes, evBrowserClose               ,277)


    END_EVENTS(MapEventTypes, 1500)
};
} // end of SEGSEvents namespace

