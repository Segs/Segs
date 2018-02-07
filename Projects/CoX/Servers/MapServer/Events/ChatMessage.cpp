#define _USE_MATH_DEFINES
#define DEBUG_INPUT
#include "ChatMessage.h"
#include "Events/InputState.h"
#include "Entity.h"
#include "MapClient.h"
#include "Servers/MapServer/DataHelpers.h"

#include <cmath>

void ChatMessage::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(10,m_source_player_id);
    bs.StorePackedBits(3,uint32_t(m_channel_type));
    bs.StoreString(m_msg);
}

void ChatMessage::serializefrom(BitStream &src)
{
    m_source_player_id = src.GetPackedBits(10);
    m_channel_type = static_cast<MessageChannel>(src.GetPackedBits(3));
    src.GetString(m_msg);
}

void sendChatMessage(MessageChannel t, QString msg, MapClient *src)
{
    ChatMessage * res = new ChatMessage(t,msg);
    res->m_source_player_id = getIdx(*src->char_entity());

    src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(res));
}
