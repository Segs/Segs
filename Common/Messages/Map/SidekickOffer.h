/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEventTypes.h"
#include "GameCommand.h"
#include "Components/BitStream.h"
#include "Components/Logging.h"

namespace SEGSEvents
{
// [[ev_def:type]]
class SidekickOffer final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    uint32_t    m_db_id;
                SidekickOffer(uint32_t db_id) : GameCommandEvent(MapEventTypes::evSidekickOffer),
                m_db_id(db_id)
                {
                }
explicit        SidekickOffer() : GameCommandEvent(MapEventTypes::evSidekickOffer) {}
    void        serializeto(BitStream &bs) const override
                {
                    bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 28
                    bs.StoreBits(32,m_db_id);
                    qCDebug(logTeams) << "Sidekick Offer db_id:" << m_db_id;
                }
    EVENT_IMPL(SidekickOffer)
};
} // end of SEGSEvents namespace

