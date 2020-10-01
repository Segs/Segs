/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Movement

// Access Level 9 Commands (GMs)
void cmdHandler_ControlsDisabled(const QStringList &params, MapClientSession &sess);
void cmdHandler_UpdateId(const QStringList &params, MapClientSession &sess);
void cmdHandler_FullUpdate(const QStringList &params, MapClientSession &sess);
void cmdHandler_HasControlId(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleInterp(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleMoveInstantly(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleCollision(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleMovementAuthority(const QStringList &params, MapClientSession &sess);
void cmdHandler_FaceEntity(const QStringList &params, MapClientSession &sess);
void cmdHandler_FaceLocation(const QStringList &params, MapClientSession &sess);
void cmdHandler_MoveZone(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleInputLog(const QStringList &params, MapClientSession &sess);
void cmdHandler_ToggleMovementLog(const QStringList &params, MapClientSession &sess);

// Access Level 2[GM] Commands
void cmdHandler_MoveTo(const QStringList &params, MapClientSession &sess);
void cmdHandler_Fly(const QStringList &params, MapClientSession &sess);
void cmdHandler_Jumppack(const QStringList &params, MapClientSession &sess);
void cmdHandler_Teleport(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_Stuck(const QStringList &params, MapClientSession &sess);
void cmdHandler_SetSpawnLocation(const QStringList &params, MapClientSession &sess);
void cmdHandler_MapXferList(const QStringList &params, MapClientSession &sess);
