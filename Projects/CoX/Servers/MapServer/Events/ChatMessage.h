/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
//TODO: those must support chaining

class GameCommand : public MapLinkEvent
{
public:
    GameCommand(size_t evtype) : MapLinkEvent(evtype) {}

    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1, 13);
        do_serialize(bs);
    }
    void serializefrom(BitStream &src)
    {
        // TODO: trouble, we need a second GameCommand Factory at this point !
        uint32_t game_command = src.GetPackedBits(1);
    }
    virtual void do_serialize(BitStream &bs) const = 0;
};

class ChatMessage : public GameCommand
{
public:
    int         m_source_player_id;
    int         m_channel_type;
    QString     m_msg;
        enum    eChatTypes
                {
                    CHAT_UNKNOWN1 = 1,
                    CHAT_UNKNOWN2,
                    CHAT_UNKNOWN3,
                    CHAT_UNKNOWN4,
                    CHAT_UNKNOWN5,
                    CHAT_UNKNOWN6,
                    CHAT_PRIVATE,
                    CHAT_TEAM,
                    CHAT_SuperGroup,
                    CHAT_Local, // Local chat
                    CHAT_Broadcast,
                    CHAT_Request,
                    CHAT_Friend,
                    CHAT_Admin = 14 // [Admin]{Message}
                };
                ChatMessage():GameCommand(MapEventTypes::evChatMessage)
                {
                }
        void    do_serialize(BitStream &bs) const;
        void    serializefrom(BitStream &src);

static  ChatMessage *adminMessage(const QString & msg);
static  ChatMessage *localMessage(const QString &msg, Entity *src);
static  ChatMessage *broadcastMessage(const QString &msg, Entity *src);
};
extern void sendAdminMessage(MapClient *tgt,const char *msg);
