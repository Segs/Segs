/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include <glm/vec3.hpp>
#include "Components/BitStream.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class SendWaypoint final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    int m_point_idx; // maybe?
    // [[ev_def:field]]
    glm::vec3 m_location;

    explicit SendWaypoint() : GameCommandEvent(evSendWaypoint) {}
    SendWaypoint(int point_idx, glm::vec3 location) : GameCommandEvent(evSendWaypoint),
        m_point_idx(point_idx),
        m_location(location)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 23
        bs.StorePackedBits(1, m_point_idx);
        bs.StoreFloat(m_location.x);
        bs.StoreFloat(m_location.y);
        bs.StoreFloat(m_location.z);
    }

    EVENT_IMPL(SendWaypoint)
};

// [[ev_def:type]]
class SetDestination final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    glm::vec3 destination;
    // [[ev_def:field]]
    int point_index;

    SetDestination():MapLinkEvent(MapEventTypes::evSetDestination)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,11);
        qWarning() << "SetDestination serializeto unimplemented.";
    }
    void serializefrom(BitStream &bs) override
    {
        destination.x = bs.GetFloat();
        destination.y = bs.GetFloat();
        destination.z = bs.GetFloat();
        point_index   = bs.GetPackedBits(1);
    }
    EVENT_IMPL(SetDestination)
};

} // end of SEGSEvents namespace
