/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommand.h"
#include "GameData/Powers.h"
#include "GameData/Trade.h"

#include <vector>

namespace SEGSEvents
{


// [[ev_def:type]]
class TradeUpdate final : public GameCommandEvent
{
public:
    EVENT_IMPL(TradeUpdate)
    TradeUpdate();
    TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other, const Entity& entity_other);
    void serializeto(BitStream& bs) const override;

    // [[ev_def:field]]
    TradeMember m_trade_self;
    // [[ev_def:field]]
    TradeMember m_trade_other;
    // [[ev_def:field]]
    std::vector<CharacterEnhancement> m_enhancements;
    // [[ev_def:field]]
    std::vector<CharacterInspiration> m_inspirations;
};


} // namespace SEGSEvents
