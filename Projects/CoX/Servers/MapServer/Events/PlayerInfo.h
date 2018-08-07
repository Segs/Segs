/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

// [[ev_def:type]]
class EntityInfoResponse final : public GameCommand
{
public:
    // [[ev_def:field]]
    QString m_info_text;
    EntityInfoResponse (const QString &txt) : GameCommand(MapEventTypes::evEntityInfoResponse),m_info_text(txt) {}
    void serializefrom(BitStream &bs);

    void serializeto(BitStream &bs) const override
    {
        qCDebug(logMapEvents) << "Info Response: " << m_info_text;
        bs.StorePackedBits(1, 69);
        bs.StoreString(m_info_text);
    }
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
};
