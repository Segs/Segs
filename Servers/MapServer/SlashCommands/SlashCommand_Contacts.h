/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Contacts, Tasks, Stores

// Access Level 9 Commands (GMs)
void cmdHandler_SendContactDialog(const QStringList &params, MapClientSession &sess);
void cmdHandler_SendContactDialogYesNoOk(const QStringList &params, MapClientSession &sess);
void cmdHandler_ContactStatusList(const QStringList &params, MapClientSession &sess);
void cmdHandler_AddTestTask(const QStringList &/*params*/, MapClientSession &sess);
void cmdHandler_OpenStore(const QStringList &/*params*/, MapClientSession &sess);
