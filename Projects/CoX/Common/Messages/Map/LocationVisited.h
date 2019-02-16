/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"

#include <QtCore/QString>
#include <glm/vec3.hpp>

namespace SEGSEvents
{
// [[ev_def:type]]
class LocationVisited : public MapLinkEvent
{
public:
    LocationVisited() : MapLinkEvent(MapEventTypes::evLocationVisited) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;

    // [[ev_def:field]]
    QString m_name;
    // [[ev_def:field]]
    glm::vec3 m_pos;
    EVENT_IMPL(LocationVisited)
};
} // end of SEGSEvents namespace

