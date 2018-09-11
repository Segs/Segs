/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "NetStructures/StateStorage.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class InputStateEvent : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    InputState   m_next_state;
    // [[ev_def:field]]
    BitStream    m_user_commands;

public:
    InputStateEvent() : MapLinkEvent(MapEventTypes::evInputStateEvent),
        m_user_commands(0)
    {}

    void receiveControlState(BitStream &bs);
    void extended_input(BitStream &bs);
    void serializefrom(BitStream &bs);
    void serializeto(BitStream &) const;
    void recv_client_opts(BitStream &bs);
    glm::vec3 &pyr() { return m_next_state.m_camera_pyr; }

    EVENT_IMPL(InputStateEvent)
};

} // end of SEGSEvents namespace
