/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <string>
#include "MapEvents.h"
#include "MapLink.h"
//TODO: those must support chaining

class GameCommand : public MapLinkEvent {
public:
                GameCommand(size_t evtype) : MapLinkEvent(evtype) {}

        void    serializeto(BitStream &bs) const
                {
                    bs.StorePackedBits(1,13);
                    do_serialize(bs);
                }
        void    serializefrom(BitStream &src)
                {
                    //TODO: trouble, we need a second GameCommand Factory at this point !
                    uint32_t game_command = src.GetPackedBits(1);
                }
virtual void    do_serialize(BitStream &bs) const =0;

};
class ChatMessage : public GameCommand
{
        int         m_source_player_id;
        signed int  m_channel_type;
        std::string m_msg;
public:
        enum        eChatTypes
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
                        CHAT_UNKNOWN10,
                        CHAT_Broadcast,
                        CHAT_Request,
                        CHAT_Friend,
                        CHAT_Admin = 14 // [Admin]{Message}
                    };
                    ChatMessage():GameCommand(MapEventTypes::evChatMessage)
                    {
                    }
        void        do_serialize(BitStream &bs) const
                    {
                            bs.StorePackedBits(1,20);
                        bs.StorePackedBits(10,m_source_player_id);
                        bs.StorePackedBits(3,m_channel_type);
                        bs.StoreString(m_msg);
                            bs.StorePackedBits(1,0); // no messages follow
                    }
        void        serializefrom(BitStream &src)
                    {
                        m_source_player_id = src.GetPackedBits(10);
                        m_channel_type = src.GetPackedBits(3);
                        src.GetString(m_msg);
                    }
static  ChatMessage *adminMessage(const char *msg)
                    {
                        ChatMessage * res = new ChatMessage;
                        res->m_channel_type = CHAT_Admin;
                        res->m_source_player_id=1;
                        res->m_msg = msg;
                        return res;
                    }
};
