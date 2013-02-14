/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <string>
#include "MapLink.h"

class ChatMessage : public MapLinkEvent
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
                    CHAT_Admin
                };
                ChatMessage():MapLinkEvent(MapEventTypes::evChatMessage)
                {
                }
    void        serializeto(BitStream &bs) const
                {
                    bs.StorePackedBits(10,m_source_player_id);
                    bs.StorePackedBits(3,m_channel_type);
                    bs.StoreString(m_msg);
                }
    void        serializefrom(BitStream &src)
                {
                    m_source_player_id = src.GetPackedBits(10);
                    m_channel_type = src.GetPackedBits(3);
                    src.GetString(m_msg);
                }
};
