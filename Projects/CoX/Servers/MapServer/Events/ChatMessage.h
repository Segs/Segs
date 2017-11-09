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
                    CHAT_UNKNOWN1 = 1, //COMBAT
                    CHAT_UNKNOWN2,     //DAMAGE
                    CHAT_UNKNOWN3,     //SVR_COM
                    CHAT_UNKNOWN4,     //NPC_SAYS
                    CHAT_UNKNOWN5,     //VILLAIN_SAYS
                    CHAT_UNKNOWN6,     //REGULAR
                    CHAT_PRIVATE,
                    CHAT_TEAM,
                    CHAT_SuperGroup,
                    CHAT_Local = 10, // Local chat
                    CHAT_Broadcast,  // shout
                    CHAT_Request,    //
                    CHAT_Friend,
                    CHAT_Admin = 14, // [Admin]{Message}
                    CHAT_UserError,
                    CHAT_DebugInfo,
                    CHAT_Emote,

                };
                ChatMessage() : GameCommand(MapEventTypes::evChatMessage)
                {
                }
        void    serializeto(BitStream &bs) const override;
        void    serializefrom(BitStream &src);

static  ChatMessage *adminMessage(const QString & msg);
static  ChatMessage *localMessage(const QString &msg, Entity *src);
static  ChatMessage *broadcastMessage(const QString &msg, Entity *src);
};
extern void sendAdminMessage(MapClient *tgt,const char *msg);
