/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class ChatDividerMoved : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    float m_position;

    ChatDividerMoved() : MapLinkEvent(MapEventTypes::evChatDividerMoved) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;
    EVENT_IMPL(ChatDividerMoved)
};

} // end of SEGSEvents namespace

