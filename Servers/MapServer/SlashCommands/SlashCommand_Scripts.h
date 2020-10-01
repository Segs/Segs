/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to ScriptEngine and Script Files

// Access Level 9 Commands
void cmdHandler_Script(const QStringList &params, MapClientSession &sess);
void cmdHandler_SmileX(const QStringList &params, MapClientSession &sess);
void cmdHandler_ReloadScripts(const QStringList &/*params*/, MapClientSession &sess);
