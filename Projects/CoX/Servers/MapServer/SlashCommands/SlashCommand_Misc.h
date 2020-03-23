/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QtCore/QString>

struct MapClientSession;

// Misc. Slash Commands

// Access Level 2[GM] Commands
uint cmdHandler_AddNPC(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_WhoAll(const QStringList &params, MapClientSession &sess);
void cmdHandler_MOTD(const QStringList &params, MapClientSession &sess);
void cmdHandler_Tailor(const QStringList &params, MapClientSession &sess);
void cmdHandler_CostumeChange(const QStringList &params, MapClientSession &sess);
void cmdHandler_Train(const QStringList &params, MapClientSession &sess);
void cmdHandler_Kiosk(const QStringList &params, MapClientSession &sess);
