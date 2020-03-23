/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to SuperGroups

// Access Level 9 Commands (GMs)
void cmdHandler_SetSuperGroup(const QStringList &params, MapClientSession &sess);
void cmdHandler_RegisterSuperGroup(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupCostume(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_SuperGroupStats(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupLeave(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupInvite(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupKick(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupMOTD(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupMotto(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupMode(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupPromote(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupDemote(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupTitles(const QStringList &params, MapClientSession &sess);

// Access Level 0 Commands
void cmdHandler_SuperGroupAccept(const QStringList &params, MapClientSession &sess);
void cmdHandler_SuperGroupDecline(const QStringList &params, MapClientSession &sess);
