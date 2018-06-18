/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "MessageChannels.h"

#include <QString>

struct MapClientSession;

class ChatMessage : public GameCommand
{
public:
    QString         m_msg;
    MessageChannel  m_channel_type;
    int             m_source_player_id;
    int             m_target_player_id;
    virtual         ~ChatMessage() = default;
                    ChatMessage(MessageChannel t, const QString &msg) : GameCommand(MapEventTypes::evChatMessage),
                        m_msg(msg),m_channel_type(t)
                    {
                    }
    void            serializeto(BitStream &bs) const override;
    void            serializefrom(BitStream &src);
};

extern void sendChatMessage(MessageChannel t, QString msg, MapClientSession *src, MapClientSession *tgt);
