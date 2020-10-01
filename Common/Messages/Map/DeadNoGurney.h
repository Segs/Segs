/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEvents.h"

namespace SEGSEvents
{

    // [[ev_def:type]]
    class DeadNoGurney : public GameCommandEvent
    {
    public:
        DeadNoGurney() : GameCommandEvent(MapEventTypes::evDeadNoGurney) {}

        // SerializableEvent interface
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // pkt 51
        }
        EVENT_IMPL(DeadNoGurney)
    };

    // [[ev_def:type]]
    class AwaitingDeadNoGurney final : public MapLinkEvent
    {
    public:
        AwaitingDeadNoGurney() : MapLinkEvent(MapEventTypes::evAwaitingDeadNoGurney)
        {}
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,10); // opcode
        }
        void serializefrom(BitStream &) override
        {
        }
        EVENT_IMPL(AwaitingDeadNoGurney)
    };

    // [[ev_def:type]]
    class DeadNoGurneyOK final : public MapLinkEvent
    {
    public:
        DeadNoGurneyOK() : MapLinkEvent(MapEventTypes::evDeadNoGurneyOK)
        {}
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,35); // opcode
        }
        void serializefrom(BitStream &) override
        {
        }
        EVENT_IMPL(DeadNoGurneyOK)
    };

} // end of namespace SEGSEvents
