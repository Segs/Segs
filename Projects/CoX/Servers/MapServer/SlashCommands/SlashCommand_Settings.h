/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Settings and Logs

// Access Level 1 Commands
void cmdHandler_SettingsDump(const QStringList &params, MapClientSession &sess);
void cmdHandler_GUIDebug(const QStringList &, MapClientSession &sess);
void cmdHandler_SetWindowVisibility(const QStringList &params, MapClientSession &sess);
void cmdHandler_KeybindDebug(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleLogging(const QStringList &params, MapClientSession &sess);
