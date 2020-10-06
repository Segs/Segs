/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameData/Powers.h"
#include "GameCommand.h"
#include "Components/BitStream.h"
#include "MapEventTypes.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class SendStance final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    PowerStance m_stance;

explicit SendStance() : GameCommandEvent(evSendStance) {}
    SendStance(PowerStance pow_stance) : GameCommandEvent(evSendStance),
        m_stance(pow_stance)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // pkt 57

        bs.StoreBits(1, m_stance.has_stance);
        if(!m_stance.has_stance)
            return;

        bs.StorePackedBits(4, m_stance.pset_idx);
        bs.StorePackedBits(4, m_stance.pow_idx);
    }
    EVENT_IMPL(SendStance)
};

// [[ev_def:type]]
class ChangeStance final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    PowerStance m_stance;

    ChangeStance():MapLinkEvent(MapEventTypes::evChangeStance)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,36);
    }
    void serializefrom(BitStream &bs) override
    {
        m_stance.has_stance = bs.GetBits(1);
        if(!m_stance.has_stance)
            return;
        m_stance.pset_idx = bs.GetPackedBits(4);
        m_stance.pow_idx  = bs.GetPackedBits(4);
    }
    EVENT_IMPL(ChangeStance)
};

} //end of SEGSEvents namespace
