/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
#include <glm/vec3.hpp>

class LocationVisited : public MapLinkEvent
{
public:
    LocationVisited() : MapLinkEvent(MapEventTypes::evLocationVisited) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;

    QString m_name;
    glm::vec3 m_pos;
};
