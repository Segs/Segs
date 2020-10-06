/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameData/ClientStates.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h"
#include "GameCommand.h"

namespace SEGSEvents
{

    // [[ev_def:type]]
    class SetClientState final : public GameCommandEvent
    {
    public:
        // [[ev_def:field]]
        ClientStates    m_new_state;
        SetClientState(ClientStates new_state=ClientStates::SIMPLE) : GameCommandEvent(evSetClientState), m_new_state(new_state)
        {
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type() - evFirstServerToClient); // pkt 18
            bs.StorePackedBits(1, static_cast<uint8_t>(m_new_state));
        }

        EVENT_IMPL(SetClientState)
    };

} // end of namespace SEGSEvents
