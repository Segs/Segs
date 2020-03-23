/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Debugging

// Access Level 9 Commands (GMs)
void cmdHandler_Dialog(const QStringList &params, MapClientSession &sess);
void cmdHandler_InfoMessage(const QStringList &params, MapClientSession &sess);
void cmdHandler_DebugChar(const QStringList &params, MapClientSession &sess);
void cmdHandler_UpdateChar(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendFloatingNumbers(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetSequence(const QStringList &params, MapClientSession &sess);
void cmdHandler_AddTriggeredMove(const QStringList &params, MapClientSession &sess);
void cmdHandler_AddTimeStateLog(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetClientState(const QStringList &params, MapClientSession &sess);
void cmdHandler_LevelUpXp(const QStringList &params, MapClientSession &sess);
void cmdHandler_TestDeadNoGurney(const QStringList &params, MapClientSession &sess);
void cmdHandler_DoorMessage(const QStringList &params, MapClientSession &sess);
void cmdHandler_Browser(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendTimeUpdate(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendWaypoint(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetStateMode(const QStringList &params, MapClientSession &sess);
void cmdHandler_Revive(const QStringList &params, MapClientSession &sess);
void cmdHandler_AddCostumeSlot(const QStringList &params, MapClientSession &sess);
void cmdHandler_ForceLogout(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendLocations(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendConsoleOutput(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendConsolePrint(const QStringList &params, MapClientSession &sess);
void cmdHandler_ClearTarget(const QStringList &params, MapClientSession &sess);
void cmdHandler_StartTimer(const QStringList &params, MapClientSession &sess);

// For live value-testing
void cmdHandler_SetU1(const QStringList &params, MapClientSession &sess);
