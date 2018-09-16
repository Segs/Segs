/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Trade.h"

#include "Servers/MapServer/DataHelpers.h"
#include "Character.h"
#include "Entity.h"
#include "Logging.h"
#include "Powers.h"


namespace {

static bool hasRoomForTradeInfluence(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;

    const uint32_t inf = (getInf(*src.m_char) - info_src.m_influence) + info_tgt.m_influence;
    if (inf < info_tgt.m_influence) // Standard overflow check (only works with unsigned integers).
    {
        messageOutput(MessageChannel::SERVER, "Not enough room to accept that much influence.", src);
        return false;
    }

    return true;
}

static bool hasRoomForTradeEnhancements(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;
    const CharacterData& cd = src.m_char->m_char_data;

    const int num_enh_src = static_cast<int>(info_src.m_enhancements.size());
    const int num_enh_tgt = static_cast<int>(info_tgt.m_enhancements.size());
    const int num_enh = getNumberEnhancements(cd) - num_enh_src + num_enh_tgt;
    const int max_num_enh = getMaxNumberEnhancements(cd);
    if (num_enh > max_num_enh)
    {
        messageOutput(MessageChannel::SERVER, "Not enough room to accept that many enhancements.", src);
        return false;
    }

    return true;
}

static bool hasRoomForTradeInspirations(Entity& src)
{
    const Trade& trade = *src.m_trade;
    const TradeInfo& info_src = trade.getMember(src).m_info;
    const TradeInfo& info_tgt = trade.getOtherMember(src).m_info;
    const CharacterData& cd = src.m_char->m_char_data;

    const int num_insp_src = static_cast<int>(info_src.m_inspirations.size());
    const int num_insp_tgt = static_cast<int>(info_tgt.m_inspirations.size());
    const int num_insp = getNumberInspirations(cd) - num_insp_src + num_insp_tgt;
    const int max_num_insp = getMaxNumberInspirations(cd);
    if (num_insp > max_num_insp)
    {
        messageOutput(MessageChannel::SERVER, "Not enough room to accept that many inspirations.", src);
        return false;
    }

    return true;
}

static bool hasRoomForTrade(Entity& src)
{
    const bool ok_inf = hasRoomForTradeInfluence(src);
    const bool ok_enh = hasRoomForTradeEnhancements(src);
    const bool ok_insp = hasRoomForTradeInspirations(src);
    return (ok_inf && ok_enh && ok_insp);
}

static std::vector<CharacterEnhancement> removeTradedEnhancements(Entity& ent, const TradeInfo& info)
{
    CharacterData& cd = ent.m_char->m_char_data;

    std::vector<CharacterEnhancement> result;
    for (const uint32_t idx : info.m_enhancements)
    {
        const CharacterEnhancement* const enh = getEnhancement(ent, idx);
        if (enh == nullptr)
        {
            continue;
        }

        result.push_back(*enh);
        trashEnhancement(cd, idx);
    }

    return result;
}

static void addTradedEnhancements(Entity& ent, const std::vector<CharacterEnhancement>& enhs)
{
    CharacterData& cd = ent.m_char->m_char_data;

    for (const CharacterEnhancement& enh : enhs)
    {
        addEnhancementToChar(cd, enh);
    }
}

static std::vector<CharacterInspiration> removeTradedInspirations(Entity& ent, const TradeInfo& info)
{
    CharacterData& cd = ent.m_char->m_char_data;

    std::vector<CharacterInspiration> result;
    for (const TradeInspiration& trade_insp : info.m_inspirations)
    {
        const CharacterInspiration* const insp = getInspiration(ent, trade_insp.m_col, trade_insp.m_row);
        if (insp == nullptr)
        {
            continue;
        }

        result.push_back(*insp);
        removeInspiration(cd, trade_insp.m_col, trade_insp.m_row);
    }

    return result;
}

static void addTradedInspirations(Entity& ent, const std::vector<CharacterInspiration>& insps)
{
    CharacterData& cd = ent.m_char->m_char_data;

    for (const CharacterInspiration& insp : insps)
    {
        addInspirationToChar(cd, insp);
    }
}

static void finishTrade(Entity& src, Entity& tgt)
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

    src.m_trade.reset();
    tgt.m_trade.reset();

    sendTradeSuccess(src, tgt);

    qCDebug(logSlashCommand) << "Trade successful betweeen" << src.name() << "and" << tgt.name(); // TODO: Trade debug
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
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_a;
    }
    else
    {
        return m_member_b;
    }
}

const TradeMember& Trade::getMember(const Entity& ent) const
{
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_a;
    }
    else
    {
        return m_member_b;
    }
}

TradeMember& Trade::getOtherMember(const Entity& ent)
{
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_b;
    }
    else
    {
        return m_member_a;
    }
}

const TradeMember& Trade::getOtherMember(const Entity& ent) const
{
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_b;
    }
    else
    {
        return m_member_a;
    }
}

bool Trade::isAccepted() const {
    return (m_member_a.m_info.m_accepted && m_member_b.m_info.m_accepted);
}


// TODO: Need a logTrades log level.

void requestTrade(Entity& src, Entity& tgt)
{
    // TODO: Clean up trade when a player logs out.
    //       The trade is cleaned up, at least for the remaining player, if trade was accepted.
    //       If trade offer is still being considered, then we need to clean it up somehow.
    //         - Entity objects are never destructed (bug?). Cannot use its destructor.
    //         - If the player who sent the request logs out, then we need to cancel the offer.
    //           Will a TradeCancel event work?
    // NOTE: This will be done when moving this code into a "TradeService", similar to "FriendshipService".
    if (src.m_trade != nullptr)
    {
        const QString msg = (src.m_trade->m_invite_accepted ?
                             "You are already in a trade." :
                             "You are already considering a trade offer.");
        messageOutput(MessageChannel::SERVER, msg, src);
        qCDebug(logSlashCommand) << "Trade invite from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    if (tgt.m_trade != nullptr)
    {
        const QString msg = (tgt.m_trade->m_invite_accepted ?
                             tgt.name() + " is already in a trade." :
                             tgt.name() + " is already considering a trade offer.");
        messageOutput(MessageChannel::SERVER, msg, src);
        qCDebug(logSlashCommand) << "Trade invite from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    // Initiate a trade offer.
    src.m_trade = std::make_shared<Trade>(src, tgt);
    tgt.m_trade = src.m_trade;

    sendTradeOffer(src, tgt);

    const QString msg_src = "You sent a trade request to " + tgt.name() + ".";
    const QString msg_tgt = src.name() + " sent a trade request.";
    messageOutput(MessageChannel::SERVER, msg_src, src);
    messageOutput(MessageChannel::SERVER, msg_tgt, tgt);
    qCDebug(logSlashCommand) << src.name() << "sent a trade request to" << tgt.name(); // TODO: Trade debug
}

void acceptTrade(Entity& src, Entity& tgt)
{
    // TODO: Why does the client sometimes send a cancel request after accepting a trade?
    //       I don't know what triggers it, but if it happens the player who sends the cancel
    //       needs to log out and in again for trade to work again.
    // NOTE: It seems to be related to the AddFriend bug where you can add a friend several times.
    //       The client does not recognize the other player.

    // These checks should never trigger.
    // If they do, then something is seriously wrong with the client.
    if (src.m_trade == nullptr)
    {
        const QString msg = "You have not been sent a trade offer.";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade accept from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    if (tgt.m_trade == nullptr)
    {
        const QString msg = tgt.name() + " have not sent a trade offer.";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade accept from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    if (src.m_trade != tgt.m_trade)
    {
        const QString msg = "You are not considering a trade offer from " + tgt.name() + ".";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade accept from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    // Accept the trade.
    src.m_trade->m_invite_accepted = true;

    sendTradeInit(src, tgt);

    const QString msg_src = "You accepted the trade invite from " + tgt.name() + ".";
    const QString msg_tgt = src.name() + " accepted your trade invite.";
    messageOutput(MessageChannel::SERVER, msg_src, src);
    messageOutput(MessageChannel::SERVER, msg_tgt, tgt);
    qCDebug(logSlashCommand) << src.name() << "accepted a trade invite from" << tgt.name(); // TODO: Trade debug
}

void declineTrade(Entity& src, Entity& tgt)
{
    // These checks should never trigger.
    // If they do, then something is seriously wrong with the client.
    if (src.m_trade == nullptr)
    {
        const QString msg = "You have not been sent a trade offer.";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    if (tgt.m_trade == nullptr)
    {
        const QString msg = tgt.name() + " have not sent a trade offer.";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    if (src.m_trade != tgt.m_trade)
    {
        const QString msg = "You are not considering a trade offer from " + tgt.name() + ".";
        messageOutput(MessageChannel::SERVER, msg, src);
        qWarning() << "Trade decline from" << src.name() << "to" << tgt.name() << "failed:" << msg; // TODO: Trade debug
        return;
    }

    // Cancel the trade request.
    src.m_trade.reset();
    tgt.m_trade.reset();

    const QString msg_src = "You declined the trade invite from " + tgt.name() + ".";
    const QString msg_tgt = src.name() + " declined your trade invite.";
    messageOutput(MessageChannel::SERVER, msg_src, src);
    messageOutput(MessageChannel::SERVER, msg_tgt, tgt);
    qCDebug(logSlashCommand) << src.name() << "declined a trade invite from" << tgt.name(); // TODO: Trade debug
}

void cancelTrade(Entity& src)
{
    if (src.m_trade == nullptr)
    {
        // Trade already cancelled.
        // The client sends this many times while closing the trade window for some reason.
        return;
    }

    const uint32_t tgt_db_id = src.m_trade->getOtherMember(src).m_db_id;
    Entity* const tgt = getEntityByDBID(src.m_client, tgt_db_id);
    if (tgt == nullptr)
    {
        // Only one side left in the game.
        src.m_trade.reset();

        const QString msg = "Trade cancelled because the other player left.";
        sendTradeCancel(src, msg);

        qCDebug(logSlashCommand) << src.name() << "cancelled a trade where target has disappeared"; // TODO: Trade debug
        return;
    }

    src.m_trade.reset();
    tgt->m_trade.reset();

    const QString msg_src = "You cancelled the trade with " + tgt->name() + ".";
    const QString msg_tgt = src.name() + " canceled the trade.";
    sendTradeCancel(src, msg_src);
    sendTradeCancel(*tgt, msg_tgt);

    qCDebug(logSlashCommand) << src.name() << "cancelled a trade with" << tgt->name(); // TODO: Trade debug
}

void updateTrade(Entity& src, const TradeInfo& info)
{
    Entity* const tgt = getEntityByDBID(src.m_client, info.m_db_id);
    if (tgt == nullptr)
    {
        return;
    }

    if (src.m_trade != tgt->m_trade)
    {
        qWarning() << "Received trade update for entities not trading with each other:"
                   << src.name() << "and" << tgt->name();
        return;
    }

    if (src.m_trade == nullptr || tgt->m_trade == nullptr)
    {
        qWarning() << "Received trade update for entities not trading:" << src.name() << "and" << tgt->name();
        return;
    }

    Trade& trade = *src.m_trade;
    TradeMember& trade_src = trade.getMember(src);
    TradeMember& trade_tgt = trade.getMember(*tgt);
    const bool is_content_modified = not trade_src.m_info.isEqualContent(info);
    trade_src.m_info = info;

    // Trade acceptance must be canceled if the trade content has changed.
    if (is_content_modified)
    {
        trade_src.m_info.m_accepted = false;
        trade_tgt.m_info.m_accepted = false;
    }

    // A trade cannot be accepted if there is not enough space.
    if (trade_src.m_info.m_accepted && !hasRoomForTrade(src))
    {
        trade_src.m_info.m_accepted = false;
    }

    sendTradeUpdate(src, *tgt, trade_src, trade_tgt);
    qCDebug(logSlashCommand) << src.name() << "updated a trade with" << tgt->name(); // TODO: Trade debug

    if (trade.isAccepted())
    {
        finishTrade(src, *tgt);
    }
}

void discardTrade(Entity& ent)
{
    ent.m_trade.reset();
}

//! @}
