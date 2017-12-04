/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

class ChatMessage : public GameCommand
{
public:
    QString     m_msg;
    int         m_source_player_id;
    int         m_channel_type;
        enum    eChatTypes
                {
                    CHAT_UNKNOWN1   = 1, // COMBAT
                    CHAT_UNKNOWN2   = 2, // DAMAGE
                    CHAT_UNKNOWN3   = 3, // SVR_COM
                    CHAT_UNKNOWN4   = 4, // NPC_SAYS
                    CHAT_UNKNOWN5   = 5, // VILLAIN_SAYS
                    CHAT_UNKNOWN6   = 6, // REGULAR
                    CHAT_PRIVATE    = 7, // Tell/Private
                    CHAT_TEAM       = 8, // Group/Team
                    CHAT_SuperGroup = 9, // SuperGroup
                    CHAT_Local      = 10, // Local
                    CHAT_Broadcast  = 11, // Shout
                    CHAT_Request    = 12, // Request
                    CHAT_Friend     = 13, // Friendlist
                    CHAT_Admin      = 14, // [Admin]{Message}
                    CHAT_UserError  = 15, // User Errors
                    CHAT_DebugInfo  = 16, // Debug Info
                    CHAT_Emote      = 17, // Emotes
                    CHAT_GENERAL    = 18, // CHAT
                };
                ChatMessage() : GameCommand(MapEventTypes::evChatMessage)
                {
                }
        void    serializeto(BitStream &bs) const override;
        void    serializefrom(BitStream &src);

static  ChatMessage *combatMessage(const QString & msg);
static  ChatMessage *damageMessage(const QString & msg);
static  ChatMessage *serverMessage(const QString & msg);
static  ChatMessage *npcMessage(const QString & msg);
static  ChatMessage *villainMessage(const QString & msg);
static  ChatMessage *regularMessage(const QString & msg);
static  ChatMessage *privateMessage(const QString &msg, Entity *src, Entity *tgt);
static  ChatMessage *teamMessage(const QString &msg, Entity *src);
static  ChatMessage *supergroupMessage(const QString &msg, Entity *src);
static  ChatMessage *localMessage(const QString &msg, Entity *src);
static  ChatMessage *broadcastMessage(const QString &msg, Entity *src);
static  ChatMessage *requestMessage(const QString &msg, Entity *src);
static  ChatMessage *friendMessage(const QString &msg, Entity *src);
static  ChatMessage *adminMessage(const QString & msg);
static  ChatMessage *errorMessage(const QString & msg);
static  ChatMessage *debugMessage(const QString & msg);
static  ChatMessage *emoteMessage(const QString & msg, Entity *src);
static  ChatMessage *generalMessage(const QString & msg, Entity *src);
};

extern void sendCombatMessage(MapClient *tgt,const char *msg);
extern void sendDamageMessage(MapClient *tgt,const char *msg);
extern void sendServerMessage(MapClient *tgt,const char *msg);
extern void sendNPCMessage(MapClient *tgt,const char *msg);
extern void sendVillainMessage(MapClient *tgt,const char *msg);
extern void sendRegularMessage(MapClient *tgt,const char *msg);
extern void sendAdminMessage(MapClient *tgt,const char *msg);
extern void sendErrorMessage(MapClient *tgt,const char *msg);
extern void sendDebugMessage(MapClient *tgt,const char *msg);
