/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/Location.h"

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
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 63
            qCDebug(logMapEvents) << "SendLocations Event serializeTo. m_locations.size(): " << m_locations.size();
            bs.StorePackedBits(1, m_locations.size() - 1);

            for(const Location &location : m_locations)
            {
               bs.StoreString(location.m_location_name);
               qCDebug(logMapEvents) << "SendLocations Event serializeTo. m_location_name: " << location.m_location_name;
            }
        }

        EVENT_IMPL(SendLocations)
    };


// [[ev_def:type]]
 /*   class ReceivedLocationVisited final : public MapLinkEvent
    {

    public:
        // [[ev_def:field]]
        Location m_location;
        explicit ReceivedLocationVisited() : MapLinkEvent(MapEventTypes::evReceiveContactStatus){}

        void serializeto(BitStream &/*bs*/ /*) const override
        {
            assert(!"ReceivedLocationVisited serializeto");
        }
        void serializefrom(BitStream &bs) // packet 62
        {
            bs.GetString(m_location.m_location_name);
            m_location.m_location_coordinates.x = bs.GetFloat();
            m_location.m_location_coordinates.y = bs.GetFloat();
            m_location.m_location_coordinates.z = bs.GetFloat();

            qCDebug(logMapEvents) << "ReceivedLocationVisited Event";
        }

        EVENT_IMPL(ReceivedLocationVisited)
    };*/
}


