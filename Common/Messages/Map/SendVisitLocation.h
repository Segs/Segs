/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/VisitLocation.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class SendLocations final : public GameCommandEvent
    {

    public:
        // [[ev_def:field]
        vLocationList m_locations;
        explicit SendLocations() : GameCommandEvent(MapEventTypes::evSendLocations){}
        SendLocations(vLocationList location_list) : GameCommandEvent(MapEventTypes::evSendLocations)
        {
            m_locations = location_list;
        }

        void serializeto(BitStream &bs) const override
        {
            //Not sure what the purpose of this packet is.
            //Could be tied to glowies or plagues/location/early badges
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 63
            qCDebug(logMapEvents) << "SendLocations Event serializeTo. m_locations.size(): " << m_locations.size();
            bs.StorePackedBits(1, m_locations.size());

            for(const VisitLocation &location : m_locations)
            {
               bs.StoreString(location.m_location_name);
               qCDebug(logMapEvents) << "SendLocations Event serializeTo. m_location_name: " << location.m_location_name;
            }
        }

        EVENT_IMPL(SendLocations)
    };
}


