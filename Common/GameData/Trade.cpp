/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Trade.h"

#include "Entity.h"
#include "EntityHelpers.h"
#include "Character.h"
#include "CharacterHelpers.h"
#include "Components/Logging.h"
#include "Powers.h"

namespace {

static TradeSystemMessages hasRoomForTradeInfluence(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;

    const uint32_t inf = (getInf(*src.m_char) - info_src.m_influence) + info_tgt.m_influence;
    if(inf < info_tgt.m_influence) // Standard overflow check (only works with unsigned integers).
        return TradeSystemMessages::NOT_ENOUGH_ROOM_INFLUENCE;

    return TradeSystemMessages::SUCCESS;
}

static TradeSystemMessages hasRoomForTradeEnhancements(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;
    const CharacterData& cd = src.m_char->m_char_data;

    const int num_enh_src = static_cast<int>(info_src.m_enhancements.size());
    const int num_enh_tgt = static_cast<int>(info_tgt.m_enhancements.size());
    const int num_enh = getNumberEnhancements(cd) - num_enh_src + num_enh_tgt;
    const int max_num_enh = getMaxNumberEnhancements(cd);
    if(num_enh > max_num_enh)
        return TradeSystemMessages::NOT_ENOUGH_ROOM_ENHANCEMENTS;

    return TradeSystemMessages::SUCCESS;
}

static TradeSystemMessages hasRoomForTradeInspirations(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;
    const CharacterData& cd = src.m_char->m_char_data;

    const int num_insp_src = static_cast<int>(info_src.m_inspirations.size());
    const int num_insp_tgt = static_cast<int>(info_tgt.m_inspirations.size());
    const int num_insp = getNumberInspirations(cd) - num_insp_src + num_insp_tgt;
    const int max_num_insp = getMaxNumberInspirations(cd);
    if(num_insp > max_num_insp)
        return TradeSystemMessages::NOT_ENOUGH_ROOM_INSPIRATIONS;

    return TradeSystemMessages::SUCCESS;
}

static TradeSystemMessages hasRoomForTrade(Entity& src)
{
    TradeSystemMessages result;
    result = hasRoomForTradeInfluence(src);
    if(result != TradeSystemMessages::SUCCESS)
        return result;

    result = hasRoomForTradeEnhancements(src);
    if(result != TradeSystemMessages::SUCCESS)
        return result;

    result = hasRoomForTradeInspirations(src);
    if(result != TradeSystemMessages::SUCCESS)
        return result;

    return TradeSystemMessages::SUCCESS;
}

static std::vector<CharacterEnhancement> removeTradedEnhancements(Entity& ent, const TradeInfo& info)
{
    CharacterData& cd = ent.m_char->m_char_data;

    std::vector<CharacterEnhancement> result;
    for (const uint32_t idx : info.m_enhancements)
    {
        const CharacterEnhancement* const enh = getEnhancement(ent, idx);
        if(enh == nullptr)
            continue;

        result.push_back(*enh);
        trashEnhancement(cd, idx);
    }

    return result;
}

static void addTradedEnhancements(Entity& ent, const std::vector<CharacterEnhancement>& enhs)
{
    CharacterData& cd = ent.m_char->m_char_data;

    for (const CharacterEnhancement& enh : enhs)
        addEnhancementToChar(cd, enh);
}

static std::vector<CharacterInspiration> removeTradedInspirations(Entity& ent, const TradeInfo& info)
{
    CharacterData& cd = ent.m_char->m_char_data;

    std::vector<CharacterInspiration> result;
    for (const TradeInspiration& trade_insp : info.m_inspirations)
    {
        const CharacterInspiration* const insp = getInspiration(ent, trade_insp.m_col, trade_insp.m_row);
        if(insp == nullptr)
            continue;

        result.push_back(*insp);
        removeInspiration(cd, trade_insp.m_col, trade_insp.m_row);
    }

    return result;
}

static void addTradedInspirations(Entity& ent, const std::vector<CharacterInspiration>& insps)
{
    CharacterData& cd = ent.m_char->m_char_data;

    for (const CharacterInspiration& insp : insps)
        addInspirationToChar(cd, insp);
}

static TradeSystemMessages checkValidTradeResponse(Entity &src, Entity &tgt)
{
    // These checks should never trigger.
    // If they do, then something is seriously wrong with the client.
    if(src.m_trade == nullptr)
    {
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:"
                   << "Source sent no trade offer.";
        return TradeSystemMessages::HAS_SENT_NO_TRADE;
    }

    if(tgt.m_trade == nullptr)
    {
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:"
                   << "Target has not received a trade offer";
        return TradeSystemMessages::TGT_RECV_NO_TRADE;
    }

    if(src.m_trade != tgt.m_trade)
    {
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:"
                   << "These are not trading partners.";
        return TradeSystemMessages::SRC_RECV_NO_TRADE;
    }

    return TradeSystemMessages::TRADE_REQUEST_IS_VALID;
}

} // Anonymous namespace.


TradeInspiration::TradeInspiration(uint32_t col, uint32_t row)
    : m_col(col), m_row(row)
{
}


bool TradeInspiration::operator==(const TradeInspiration& other) const
{
    return (m_col == other.m_col && m_row == other.m_row);
}


bool TradeInfo::isEqualContent(const TradeInfo& info) const
{
    return (m_influence == info.m_influence &&
            m_enhancements == info.m_enhancements &&
            m_inspirations == info.m_inspirations);
}


TradeMember::TradeMember()
{
}


TradeMember::TradeMember(const Entity& ent)
    : m_db_id(getDbId(ent))
{
}


Trade::Trade(const Entity& ent_a, const Entity& ent_b)
    : m_member_a(ent_a),
      m_member_b(ent_b)
{
}


TradeMember& Trade::getMember(const Entity& ent)
{
    if(m_member_a.m_db_id == getDbId(ent))
        return m_member_a;

    return m_member_b;
}

const TradeMember& Trade::getMember(const Entity& ent) const
{
    if(m_member_a.m_db_id == getDbId(ent))
        return m_member_a;

    return m_member_b;
}

TradeMember& Trade::getOtherMember(const Entity& ent)
{
    if(m_member_a.m_db_id == getDbId(ent))
        return m_member_b;

    return m_member_a;
}

const TradeMember& Trade::getOtherMember(const Entity& ent) const
{
    if(m_member_a.m_db_id == getDbId(ent))
        return m_member_b;

    return m_member_a;
}

bool Trade::isAccepted() const
{
    return (m_member_a.m_info.m_accepted && m_member_b.m_info.m_accepted);
}


TradeSystemMessages requestTrade(Entity& src, Entity& tgt)
{
    // TODO: Clean up trade when a player logs out.
    //       The trade is cleaned up, at least for the remaining player, if trade was accepted.
    //       If trade offer is still being considered, then we need to clean it up somehow.
    //         - Entity objects are never destructed (bug?). Cannot use its destructor.
    //         - If the player who sent the request logs out, then we need to cancel the offer.
    //           Will a TradeCancel event work?
    // NOTE: This will be done when moving this code into a "TradeService", similar to "FriendshipService".
    if(src.m_trade != nullptr)
    {
        qCDebug(logTrades) << "Trade invite from" << src.name() << "to" << tgt.name() << "failed:"
                           << src.m_trade->m_invite_accepted;
        if(src.m_trade->m_invite_accepted)
            return TradeSystemMessages::SRC_ALREADY_IN_TRADE;
        else
            return TradeSystemMessages::SRC_CONSIDERING_ANOTHER_TRADE;
    }

    if(tgt.m_trade != nullptr)
    {
        qCDebug(logTrades) << "Trade invite from" << src.name() << "to" << tgt.name() << "failed:"
                           << tgt.m_trade->m_invite_accepted;
        if(tgt.m_trade->m_invite_accepted)
            return TradeSystemMessages::TGT_ALREADY_IN_TRADE;
        else
            return TradeSystemMessages::TGT_CONSIDERING_ANOTHER_TRADE;
    }

    // Initiate a trade offer.
    src.m_trade = std::make_shared<Trade>(src, tgt);
    tgt.m_trade = src.m_trade;

    qCDebug(logTrades) << src.name() << "sent a trade request to" << tgt.name();
    return TradeSystemMessages::SEND_TRADE_OFFER;
}

TradeSystemMessages acceptTrade(Entity& src, Entity& tgt)
{
    // TODO: Why does the client sometimes send a cancel request after accepting a trade?
    //       I don't know what triggers it, but if it happens the player who sends the cancel
    //       needs to log out and in again for trade to work again.
    // NOTE: It seems to be related to the AddFriend bug where you can add a friend several times.
    //       The client does not recognize the other player.

    TradeSystemMessages result;
    result = checkValidTradeResponse(src, tgt);
    if(result != TradeSystemMessages::TRADE_REQUEST_IS_VALID)
        return result;

    // Accept the trade.
    src.m_trade->m_invite_accepted = true;

    qCDebug(logTrades) << src.name() << "accepted a trade invite from" << tgt.name();
    return TradeSystemMessages::ACCEPTED_TRADE;
}

TradeSystemMessages declineTrade(Entity& src, Entity& tgt)
{
    TradeSystemMessages result;
    result = checkValidTradeResponse(src, tgt);
    if(result != TradeSystemMessages::TRADE_REQUEST_IS_VALID)
        return result;

    // Cancel the trade request.
    discardTrade(src);
    discardTrade(tgt);

    qCDebug(logTrades) << src.name() << "declined a trade invite from" << tgt.name();
    return TradeSystemMessages::DECLINED_TRADE;
}

TradeSystemMessages updateTrade(Entity &src, Entity &tgt, const TradeInfo &info)
{
    if(src.m_trade != tgt.m_trade)
    {
        qWarning() << "Received trade update for entities not trading with each other:"
                   << src.name() << "and" << tgt.name();
        return TradeSystemMessages::GENERIC_FAILURE;
    }

    if(src.m_trade == nullptr || tgt.m_trade == nullptr)
    {
        qWarning() << "Received trade update for entities not trading:" << src.name() << "and" << tgt.name();
        return TradeSystemMessages::GENERIC_FAILURE;
    }

    Trade& trade = *src.m_trade;
    TradeMember& trade_src = trade.getMember(src);
    TradeMember& trade_tgt = trade.getMember(tgt);
    const bool is_content_modified = !trade_src.m_info.isEqualContent(info);
    trade_src.m_info = info;

    // Trade acceptance must be canceled if the trade content has changed.
    if(is_content_modified)
    {
        trade_src.m_info.m_accepted = false;
        trade_tgt.m_info.m_accepted = false;
    }

    // A trade cannot be accepted if there is not enough space.
    TradeSystemMessages result;
    result = hasRoomForTrade(src);
    if(trade_src.m_info.m_accepted && result != TradeSystemMessages::SUCCESS)
    {
        trade_src.m_info.m_accepted = false;
        return result;
    }

    qCDebug(logTrades) << src.name() << "updated a trade with" << tgt.name();
    return TradeSystemMessages::SUCCESS; // sendTradeUpdate() in MapInstance
}

void discardTrade(Entity& ent)
{
    ent.m_trade.reset();
}

void finishTrade(Entity& src, Entity& tgt)
{
    Trade& trade = *src.m_trade;
    TradeInfo& info_src = trade.getMember(src).m_info;
    TradeInfo& info_tgt = trade.getMember(tgt).m_info;

    // Influence.
    const uint32_t inf_src = (getInf(*src.m_char) - info_src.m_influence) + info_tgt.m_influence;
    const uint32_t inf_tgt = (getInf(*tgt.m_char) - info_tgt.m_influence) + info_src.m_influence;
    setInf(*src.m_char, inf_src);
    setInf(*tgt.m_char, inf_tgt);

    // Enhancements.
    const std::vector<CharacterEnhancement> enhs_src = removeTradedEnhancements(src, info_src);
    const std::vector<CharacterEnhancement> enhs_tgt = removeTradedEnhancements(tgt, info_tgt);
    addTradedEnhancements(src, enhs_tgt);
    addTradedEnhancements(tgt, enhs_src);

    // Inspirations.
    const std::vector<CharacterInspiration> insps_src = removeTradedInspirations(src, info_src);
    const std::vector<CharacterInspiration> insps_tgt = removeTradedInspirations(tgt, info_tgt);
    addTradedInspirations(src, insps_tgt);
    addTradedInspirations(tgt, insps_src);

    discardTrade(src);
    discardTrade(tgt);

    qCDebug(logTrades) << "Trade successful betweeen" << src.name() << "and" << tgt.name();
}

//! @}
