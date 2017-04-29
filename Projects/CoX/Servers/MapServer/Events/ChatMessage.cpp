#define _USE_MATH_DEFINES
#define DEBUG_INPUT
#include <cmath>
#include "Events/InputState.h"
#include "Entity.h"



void ChatMessage::do_serialize(BitStream &bs) const
{
    bs.StorePackedBits(1,20);
    bs.StorePackedBits(10,m_source_player_id);
    bs.StorePackedBits(3,m_channel_type);
    bs.StoreString(m_msg);
    bs.StorePackedBits(1,0); // no messages follow
}

void ChatMessage::serializefrom(BitStream &src)
{
    m_source_player_id = src.GetPackedBits(10);
    m_channel_type = src.GetPackedBits(3);
    src.GetString(m_msg);
}

ChatMessage *ChatMessage::adminMessage(const char *msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Admin;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::localMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Broadcast;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}
