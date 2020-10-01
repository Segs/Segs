/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommand.h"


namespace SEGSEvents
{


// [[ev_def:type]]
class TradeInit final : public GameCommandEvent
{
public:
    EVENT_IMPL(TradeInit)
    TradeInit();
    explicit TradeInit(uint32_t db_id);
    void serializeto(BitStream& bs) const override;

    // [[ev_def:field]]
    uint32_t m_db_id = 0;
};


} // namespace SEGSEvents
