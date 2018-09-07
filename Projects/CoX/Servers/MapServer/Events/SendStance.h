/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Powers.h"
#include "GameCommand.h"
#include "MapEventTypes.h"

#include <QtCore/QString>

namespace SEGSEvents
{
// [[ev_def:type]]
class SendStance final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    PowerStance m_stance;

explicit SendStance() : GameCommandEvent(MapEventTypes::evSendStance) {}
    SendStance(PowerStance pow_stance) : GameCommandEvent(MapEventTypes::evSendStance),
        m_stance(pow_stance)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient); // 57

        bs.StoreBits(1, m_stance.has_stance);
        if(!m_stance.has_stance)
            return;

        bs.StorePackedBits(4, m_stance.pset_idx);
        bs.StorePackedBits(4, m_stance.pow_idx);
    }
    EVENT_IMPL(SendStance)
};
} //end of SEGSEvents namespace
