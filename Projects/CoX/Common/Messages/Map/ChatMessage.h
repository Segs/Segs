/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MessageChannels.h"
#include "GameCommand.h"
#include "MapEventTypes.h"

struct MapClientSession;

namespace SEGSEvents
{
// [[ev_def:type]]
class ChatMessage : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString         m_msg;
    // [[ev_def:field]]
    MessageChannel  m_channel_type;
    // [[ev_def:field]]
    int             m_source_player_id;
    // [[ev_def:field]]
    int             m_target_player_id;
                    ChatMessage() : GameCommandEvent(MapEventTypes::evChatMessage) {}
                    ChatMessage(MessageChannel t, const QString &msg) : GameCommandEvent(MapEventTypes::evChatMessage),
                        m_msg(msg),m_channel_type(t)
                    {
                    }
    void            serializeto(BitStream &bs) const override;
    void            serializefrom(BitStream &src) override;
    EVENT_IMPL(ChatMessage)
};

} // end of SEGSEvents namespace
