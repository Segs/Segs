/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Teams, Sidekicks, and LFG

// Access Level 9 Commands (GMs)
void cmdHandler_SetTeam(const QStringList &params, MapClientSession &sess);
void cmdHandler_TeamDebug(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_Invite(const QStringList &params, MapClientSession &sess);
void cmdHandler_Kick(const QStringList &params, MapClientSession &sess);
void cmdHandler_LeaveTeam(const QStringList &params, MapClientSession &sess);
void cmdHandler_FindMember(const QStringList &params, MapClientSession &sess);
void cmdHandler_MakeLeader(const QStringList &params, MapClientSession &sess);
void cmdHandler_TeamBuffs(const QStringList &params, MapClientSession &sess);
void cmdHandler_Sidekick(const QStringList &params, MapClientSession &sess);
void cmdHandler_UnSidekick(const QStringList &params, MapClientSession &sess);
void cmdHandler_LFG(const QStringList &params, MapClientSession &sess);

// Access Level 0 Commands
void cmdHandler_TeamAccept(const QStringList &params, MapClientSession &sess);
void cmdHandler_TeamDecline(const QStringList &params, MapClientSession &sess);
void cmdHandler_SidekickAccept(const QStringList &params, MapClientSession &sess);
void cmdHandler_SidekickDecline(const QStringList &params, MapClientSession &sess);
