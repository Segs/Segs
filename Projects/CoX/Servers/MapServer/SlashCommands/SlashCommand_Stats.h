/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Character Stats or values

// Access Level 9 Commands (GMs)
void cmdHandler_Falling(const QStringList &params, MapClientSession &sess);
void cmdHandler_Sliding(const QStringList &params, MapClientSession &sess);
void cmdHandler_Jumping(const QStringList &params, MapClientSession &sess);
void cmdHandler_Stunned(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetSpeed(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetBackupSpd(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetJumpHeight(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetHP(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetEnd(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetXP(const QStringList &params, MapClientSession &sess);
void cmdHandler_GiveXP(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetDebt(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetInf(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetLevel(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetCombatLevel(const QStringList &params, MapClientSession &sess);

// Access Level 2[GM] Commands
void cmdHandler_Alignment(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_AFK(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetTitles(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetCustomTitles(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetSpecialTitle(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetAssistTarget(const QStringList &params, MapClientSession &sess);
