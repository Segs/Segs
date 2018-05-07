/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "MapEvents.h"
//#include "NetStructures/Entity.h"
#include "NetStructures/InputStates.h"

class InputStateEvent : public MapLinkEvent
{
public:
    InputState   m_current;
    BitStream    m_user_commands;

public:
    InputStateEvent() : MapLinkEvent(MapEventTypes::evInputState),m_user_commands(0)
    {}

    void processDirectionControl(uint8_t dir, int prev_time, int press_release);
    void receiveInputStateHistory(BitStream &bs);
    void extended_input(BitStream &bs);
    void serializefrom(BitStream &bs);
    void serializeto(BitStream &) const;
    void recv_client_opts(BitStream &bs);
    glm::vec3 &pyr() {return m_current.m_camera_pyr;}
};
