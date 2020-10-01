/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

enum class MessageChannel : int
{
    COMBAT         = 1, // COMBAT
    DAMAGE         = 2, // DAMAGE
    SERVER         = 3, // SVR_COM
    NPC_SAYS       = 4, // NPC_SAYS
    VILLAIN_SAYS   = 5, // VILLAIN_SAYS
    REGULAR        = 6, // REGULAR
    PRIVATE        = 7, // Tell/Private
    TEAM           = 8, // Group/Team
    SUPERGROUP     = 9, // SuperGroup
    LOCAL          = 10, // Local
    BROADCAST      = 11, // Shout
    REQUEST        = 12, // Request
    FRIENDS        = 13, // Friendlist
    ADMIN          = 14, // [Admin]{Message}
    USER_ERROR     = 15, // User Errors
    DEBUG_INFO     = 16, // Debug Info
    EMOTE          = 17, // Emotes
    CHAT_TEXT      = 18, // General CHAT
    PROFILE_TEXT   = 19, // Profile Text; unused?
    HELP_TEXT      = 20, // Help Text; unused?
    STD_TEXT       = 21, // Standard Text; unused?
};
