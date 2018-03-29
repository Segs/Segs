/*
 * Super Entity Game Server Project
 * http://github.com/Segs
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"
#include "MessageChannels.h"

#include <QString>

class MapClient;

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
