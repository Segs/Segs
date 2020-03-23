/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Emails

// Access Level 0 Commands
void cmdHandler_EmailHeaders(const QStringList &params, MapClientSession &sess);
void cmdHandler_EmailRead(const QStringList &params, MapClientSession &sess);
void cmdHandler_EmailSend(const QStringList &params, MapClientSession &sess);
void cmdHandler_EmailDelete(const QStringList &params, MapClientSession &sess);
