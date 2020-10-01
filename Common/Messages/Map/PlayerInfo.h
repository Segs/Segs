/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"


namespace SEGSEvents
{
// [[ev_def:type]]
class EntityInfoResponse final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString m_info_text;
    explicit EntityInfoResponse () : GameCommandEvent(MapEventTypes::evEntityInfoResponse)
    {
    }
    EntityInfoResponse (const QString &txt) : GameCommandEvent(MapEventTypes::evEntityInfoResponse),m_info_text(txt)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        qCDebug(logMapEvents) << "Info Response: " << m_info_text;
        bs.StorePackedBits(1, 69);
        bs.StoreString(m_info_text);
    }
    EVENT_IMPL(EntityInfoResponse)
};

// [[ev_def:type]]
class EntityInfoRequest final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int entity_idx;
    EntityInfoRequest():MapLinkEvent(MapEventTypes::evEntityInfoRequest)
    {}
    void serializeto(BitStream &/*bs*/) const override
    {
        assert(!"implemented");
    }
    void serializefrom(BitStream &bs) override
    {
        entity_idx = bs.GetPackedBits(12);
    }
    EVENT_IMPL(EntityInfoRequest)
};
} // end of SEGSEvents namespace

