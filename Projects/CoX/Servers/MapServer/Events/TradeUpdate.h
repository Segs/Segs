/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommandList.h"

class TradeMember;

class TradeUpdate final : public GameCommand
{
public:
    TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other);
    virtual ~TradeUpdate() override = default;
    virtual void serializeto(BitStream& bs) const override;

    const uint32_t m_db_id;
    const bool m_self_accepted;
    const bool m_other_accepted;
    const uint32_t m_self_influence;
    const uint32_t m_other_influence;
    // TODO: Inspirations and enhancements.
};
