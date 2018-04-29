/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
#include <glm/vec3.hpp>

class PlaqueVisited : public MapLinkEvent
{
public:
    PlaqueVisited() : MapLinkEvent(MapEventTypes::evPlaqueVisited) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;

    QString m_name;
    glm::vec3 m_pos;
};
