/*
 * Super Entity Game Server Project
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "MapEvents.h"
#include "MapLink.h"

class QString;

class ChatDividerMoved : public MapLinkEvent
{
public:
    float m_position;
    ChatDividerMoved() : MapLinkEvent(MapEventTypes::evChatDividerMoved) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;
};
