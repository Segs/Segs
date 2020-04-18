/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2020 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

struct MapClientSession;
class QStringList;

// Slash Commands related to Friends and Friendslist

// Access Level 9 Commands (GMs)
void cmdHandler_FriendsListDebug(const QStringList &params, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_Friend(const QStringList &params, MapClientSession &sess);
void cmdHandler_Unfriend(const QStringList &params, MapClientSession &sess);
void cmdHandler_FriendList(const QStringList &params, MapClientSession &sess);
