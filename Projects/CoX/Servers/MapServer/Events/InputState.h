/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
#include "NetStructures/Entity.h"
namespace SEGSEvents
{

// [[ev_def:type]]
class InputState : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    InputStateStorage   m_data;
    // [[ev_def:field]]
    bool                m_has_target;
    // [[ev_def:field]]
    uint32_t            m_target_idx;
    // [[ev_def:field]]
    BitStream m_user_commands;

public:
    InputState() : MapLinkEvent(MapEventTypes::evInputState),m_user_commands(0)
    {}
    void serializeto(BitStream &) const;
    void serializefrom(BitStream &bs);
    void partial_2(BitStream &bs);
    void extended_input(BitStream &bs);
    void recv_client_opts(BitStream &bs);
    glm::vec3 &pyr() {return m_data.m_camera_pyr;}
};
} // end of SEGSEvents namespace

