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
