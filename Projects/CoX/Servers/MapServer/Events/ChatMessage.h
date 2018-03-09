/*
 * Super Entity Game Server Project
 * http://github.com/Segs
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

enum class MessageChannel : int {
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

class ChatMessage : public GameCommand
{
public:
    QString         m_msg;
    MessageChannel  m_channel_type;
    int             m_source_player_id;
    int             m_target_player_id;
                    ChatMessage(MessageChannel t, QString msg) : GameCommand(MapEventTypes::evChatMessage),
                        m_channel_type(t),m_msg(msg)
                    {
                    }
    void            serializeto(BitStream &bs) const override;
    void            serializefrom(BitStream &src);
};

extern void sendChatMessage(MessageChannel t, QString msg, MapClient *src, MapClient *tgt);
