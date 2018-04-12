#define _USE_MATH_DEFINES
#include "ChatMessage.h"
#include "Events/InputState.h"
#include "Entity.h"
#include "MapClient.h"
#include "MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Logging.h"

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

void sendChatMessage(MessageChannel t, QString msg, MapClientSession *src, MapClientSession *tgt)
{
    ChatMessage * res = new ChatMessage(t,msg);
    res->m_source_player_id = getIdx(*src->char_entity());
    res->m_target_player_id = getIdx(*src->char_entity());

    tgt->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(res));

    qCDebug(logChat).noquote() << "ChatMessage:"
             << "\n  Channel:" << int(res->m_channel_type)
             << "\n  Source:" << res->m_source_player_id
             << "\n  Target:" << res->m_target_player_id
             << "\n  Message:" << res->m_msg;
}
