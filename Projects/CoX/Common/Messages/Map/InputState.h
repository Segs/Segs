/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "GameData/StateStorage.h"
#include "GameData/Entity.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class RecvInputState : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    InputStateChange m_input_state_change;
    // [[ev_def:field]]
    BitStream m_user_commands;

public:
    RecvInputState() : MapLinkEvent(MapEventTypes::evRecvInputState),m_user_commands(0)
    {}
    void serializeto(BitStream &) const override;
    void serializefrom(BitStream &bs) override;
    void receiveControlStateChanges(BitStream &bs);
    void extended_input(BitStream &bs);
    void recv_client_opts(BitStream &bs);
    EVENT_IMPL(RecvInputState)
};

} // end of SEGSEvents namespace

