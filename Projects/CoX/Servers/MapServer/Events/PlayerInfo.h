/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

class EntityInfoResponse final : public MapLinkEvent
{
public:
    QString m_info_text;
    EntityInfoResponse (const QString &txt) : MapLinkEvent(MapEventTypes::evEntityInfoResponse),m_info_text(txt) {}
    void serializefrom(BitStream &bs) override
    {
        assert(!"implemented");
    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 69);
        bs.StoreString(m_info_text);
    }
};

class EntityInfoRequest final : public MapLinkEvent
{
public:
    int entity_idx;
    EntityInfoRequest():MapLinkEvent(MapEventTypes::evEntityInfoRequest)
    {}
    void serializeto(BitStream &bs) const override
    {
        assert(!"implemented");
    }
    void serializefrom(BitStream &bs) override
    {
        entity_idx = bs.GetPackedBits(12);
    }
};
