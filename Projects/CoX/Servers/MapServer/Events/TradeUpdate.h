/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommandList.h"
#include "NetStructures/Trade.h"


class TradeUpdate final : public GameCommand
{
public:
    TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other, const Entity& entity_other);
    virtual ~TradeUpdate() override = default;
    virtual void serializeto(BitStream& bs) const override;

    const TradeMember m_trade_self;
    const TradeMember m_trade_other;
    const Entity& m_entity_other;
};
