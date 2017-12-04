#define _USE_MATH_DEFINES
#define DEBUG_INPUT
#include "ChatMessage.h"
#include "Events/InputState.h"
#include "Entity.h"
#include "MapClient.h"
#include <cmath>



void ChatMessage::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(10,m_source_player_id);
    bs.StorePackedBits(3,m_channel_type);
    bs.StoreString(m_msg);
}

void ChatMessage::serializefrom(BitStream &src)
{
    m_source_player_id = src.GetPackedBits(10);
    m_channel_type = src.GetPackedBits(3);
    src.GetString(m_msg);
}

ChatMessage *ChatMessage::combatMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN1;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::damageMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN2;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::serverMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN3;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::npcMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN4;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::villainMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN5;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::regularMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UNKNOWN6;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::privateMessage(const QString &msg, Entity *src, Entity *tgt)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_PRIVATE;
    res->m_source_player_id=src->getIdx();
    //res->m_target_player_id=tgt->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::teamMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_TEAM;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::supergroupMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_SuperGroup;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::localMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Local;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::broadcastMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Broadcast;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::requestMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Request;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::friendMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Friend;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::adminMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Admin;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::errorMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_UserError;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::debugMessage(const QString &msg)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_DebugInfo;
    res->m_source_player_id=1;
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::emoteMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_Emote;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

ChatMessage *ChatMessage::generalMessage(const QString &msg, Entity *src)
{
    ChatMessage * res = new ChatMessage;
    res->m_channel_type = CHAT_GENERAL;
    res->m_source_player_id=src->getIdx();
    res->m_msg = msg;
    return res;
}

void sendCombatMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::combatMessage(msg)));
}

void sendDamageMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::damageMessage(msg)));
}

void sendServerMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::serverMessage(msg)));
}

void sendNPCMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::npcMessage(msg)));
}

void sendVillainMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::villainMessage(msg)));
}

void sendRegularMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::regularMessage(msg)));
}

void sendAdminMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::adminMessage(msg)));
}

void sendErrorMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::errorMessage(msg)));
}

void sendDebugMessage(MapClient * tgt, const char * msg)
{
    if(tgt)
        tgt->link()->putq(new PreUpdateCommand(ChatMessage::debugMessage(msg)));
}


