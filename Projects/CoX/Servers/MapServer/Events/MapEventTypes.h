/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUD_Events.h"

class MapEventTypes : public CRUD_EventTypes
{
public:
    BEGINE_EVENTS(CRUD_EventTypes)
    EVENT_DECL(evEntityEnteringMap          ,0)
    EVENT_DECL(evMapInstanceConnected       ,1)
    EVENT_DECL(evShortcutsRequest           ,2)
    EVENT_DECL(evShortcuts                  ,3)
    EVENT_DECL(evSceneRequest               ,4)
    EVENT_DECL(evScene                      ,5)
    EVENT_DECL(evEntitiesRequest            ,6)
    EVENT_DECL(evEntitites                  ,7)
    EVENT_DECL(evInputState                 ,8)
    EVENT_DECL(evClientQuit                 ,9)
    EVENT_DECL(evForceLogout                ,10)
    EVENT_DECL(evChatDividerMoved           ,36)
    EVENT_DECL(evPreUpdateCommand           ,1013)  // this wraps a list of commands
// server -> client commands
    EVENT_DECL(evFirstServerToClient        ,100)
//    EVENT_DECL(evServerGameCommand        ,113) // 1013?
    EVENT_DECL(evStandardDialogCmd          ,114)
    EVENT_DECL(evInfoMessageCmd             ,115)
//    EVENT_DECL(evDoorMsg                  ,116)
//    EVENT_DECL(evConsoleOutput            ,117)
    EVENT_DECL(evSetClientState             ,118)
//    EVENT_DECL(evConPrintf                ,119)
    EVENT_DECL(evChatMessage                ,120)
    EVENT_DECL(evFloatingDamage             ,121)
//    EVENT_DECL(evVisitMapCells            ,122)
//    EVENT_DECL(evSendWaypoint             ,123)
    EVENT_DECL(evTeamOffer                  ,124)
    EVENT_DECL(evTeamLooking                ,125)
//    EVENT_DECL(evTaskForceKick            ,126)
//    EVENT_DECL(evTaskForceQuit            ,127)
    EVENT_DECL(evSidekickOffer              ,128)
//    EVENT_DECL(evSuperGroupOffer          ,129)
//    EVENT_DECL(evSuperGroupResponse       ,130)
//    EVENT_DECL(evSuperGroupCostume        ,131)
//    EVENT_DECL(evTradeOffer               ,132)
//    EVENT_DECL(evTradeInit                ,133)
//    EVENT_DECL(evTradeCancel              ,134)
//    EVENT_DECL(evTradeUpdate              ,135)
//    EVENT_DECL(evTradeSuccess             ,136)
    EVENT_DECL(evFriendListUpdated          ,137)
//    EVENT_DECL(evControlPlayer            ,138)
//    EVENT_DECL(evContactDialog            ,139)
//    EVENT_DECL(evContactDialogClose       ,140)
//    EVENT_DECL(evContactDialogOk          ,141)
//    EVENT_DECL(evContactDialogYesNo       ,142)
//    EVENT_DECL(evContactStatusList        ,143)
//    EVENT_DECL(evContactSelect            ,144)
//    EVENT_DECL(evTaskStatusList           ,145)
//    EVENT_DECL(evTaskSelect               ,146)
//    EVENT_DECL(evTaskRemoveTeammates      ,147)
//    EVENT_DECL(evTimeUpdate               ,148)
//    EVENT_DECL(evMissionEntryText         ,149)
//    EVENT_DECL(evMissionKick              ,150)
//    EVENT_DECL(evDeadNoGurney             ,151)
//    EVENT_DECL(evDoorAnimStart            ,152)
//    EVENT_DECL(evDoorAnimExit             ,153)
//    EVENT_DECL(evFaceEntity               ,154)
//    EVENT_DECL(evFaceLocation             ,155)
    EVENT_DECL(evFloatingInfo               ,156)
    EVENT_DECL(evSendStance                 ,157) // I think?
    EVENT_DECL(evMapXferList                ,158)
    EVENT_DECL(evMapXferListClose           ,159)
//    EVENT_DECL(evEnableControlLog         ,160)
    EVENT_DECL(evLevelUp                    ,161)
    EVENT_DECL(evChangeTitle                ,162)
//    EVENT_DECL(evCSRBugReport             ,164)
    EVENT_DECL(evEmailHeadersCmd            ,165)
    EVENT_DECL(evEmailReadCmd               ,166)
    EVENT_DECL(evEmailMsgStatus             ,167)
    EVENT_DECL(evEntityInfoResponse         ,169) // Send entity info (description)
//    EVENT_DECL(evClueUpdate               ,170)
    EVENT_DECL(evTrayAdd                    ,182)
    EVENT_DECL(evCombineEnhancResponse      ,183)
// client -> server commands
    EVENT_DECL(evConsoleCommand             ,200)
    EVENT_DECL(evMiniMapState               ,201)
    EVENT_DECL(evClientResumedRendering     ,204)
    EVENT_DECL(evCookieRequest              ,206)
    EVENT_DECL(evInteractWithEntity         ,207)
    EVENT_DECL(evSwitchTray                 ,208) // Switch Tray using left-right arrows next to power tray
    EVENT_DECL(evEnterDoor                  ,209)
    EVENT_DECL(evSetDestination             ,211)
    EVENT_DECL(evWindowState                ,214)
    EVENT_DECL(evInspirationDockMode        ,217)
    EVENT_DECL(evPowersDockMode             ,218)
    EVENT_DECL(evSetKeybind                 ,219)
    EVENT_DECL(evRemoveKeybind              ,220)
    EVENT_DECL(evResetKeybinds              ,221)
    EVENT_DECL(evSelectKeybindProfile       ,222)
    EVENT_DECL(evDialogButton               ,224)
    EVENT_DECL(evActivatePower              ,227)
    EVENT_DECL(evActivatePowerAtLocation    ,228)
    EVENT_DECL(evActivateInspiration        ,229) // Hit F1, F2, F3, F4, F5 or Click on Inspiration in Tray
    EVENT_DECL(evSetDefaultPowerSend        ,230) // Send Default Powerset
    EVENT_DECL(evSetDefaultPower            ,231) // Clicking 'z'
    EVENT_DECL(evUnqueueAll                 ,232) // Click on another entity or Clicking 'z'
    EVENT_DECL(evAbortQueuedPower           ,233) // Clicking 'z'
    EVENT_DECL(evMoveInspiration            ,234)
    EVENT_DECL(evChangeStance               ,236)
    EVENT_DECL(evTargetChatChannelSelected  ,237) // Change chat type using the letters in the chat window 'l, b, g, sg, r, f"
    EVENT_DECL(evChatReconfigure            ,238)
    EVENT_DECL(evPlaqueVisited              ,239)
    EVENT_DECL(evCombineEnhancements        ,240)
    EVENT_DECL(evMoveEnhancement            ,241)
    EVENT_DECL(evSetEnhancement             ,242)
    EVENT_DECL(evTrashEnhancement           ,243)
    EVENT_DECL(evRecvNewPower               ,246)
    EVENT_DECL(evEntityInfoRequest          ,256) // right-click entity select "Info"
    EVENT_DECL(evLocationVisited            ,262)
    EVENT_DECL(evSwitchViewPoint            ,264)
    EVENT_DECL(evSaveClientOptions          ,265)
    EVENT_DECL(evRecvSelectedTitles         ,266)
    EVENT_DECL(evDescriptionAndBattleCry    ,267)
    END_EVENTS(1500)
};
