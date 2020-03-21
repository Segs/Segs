/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"

#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class MapXferWait final : public MapLinkEvent
{
public:
    MapXferWait() : MapLinkEvent(MapEventTypes::evMapXferWait)
    {}
    MapXferWait(QString map_name) : MapLinkEvent(MapEventTypes::evMapXferWait)
    {
        m_map_name = map_name;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 10);  // opcode
        bs.StoreString(m_map_name);
    }
    void serializefrom(BitStream &/*src*/) override
    {
    }

    // [[ev_def:field]]
    QString     m_map_name;

    EVENT_IMPL(MapXferWait)
};

} //end of SEGSEvents namespace
