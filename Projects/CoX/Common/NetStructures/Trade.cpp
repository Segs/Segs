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
#include "Entity.h"
#include "Logging.h"


namespace {

static void finishTrade(Entity& src, Entity& tgt)
{
    Trade& trade = *src.m_trade;
    TradeMember& trade_src = trade.getMember(src);
    TradeMember& trade_tgt = trade.getMember(tgt);

    const uint32_t inf_src = getInf(*src.m_char) + trade_tgt.m_influence - trade_src.m_influence;
    const uint32_t inf_tgt = getInf(*tgt.m_char) + trade_src.m_influence - trade_tgt.m_influence;
    setInf(*src.m_char, inf_src);
    setInf(*tgt.m_char, inf_tgt);
    // TODO: Inspirations and enhancements.

    src.m_trade.reset();
    tgt.m_trade.reset();

    sendTradeSuccess(src, tgt);

    qCDebug(logSlashCommand) << "Trade successful betweeen" << src.name() << "and" << tgt.name(); // TODO: Trade debug
}

} // Anonymous namespace.


TradeMember::TradeMember(const Entity& ent)
    : m_db_id(getDbId(ent))
{
}


Trade::Trade(const Entity& ent_a, const Entity& ent_b)
    : m_member_a(ent_a),
      m_member_b(ent_b)
{
}


TradeMember& Trade::getMember(Entity& ent)
{
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_a;
    }
    else
    {
        return m_member_b;
    }
}

TradeMember& Trade::getOtherMember(Entity& ent)
{
    if (m_member_a.m_db_id == getDbId(ent)) {
        return m_member_b;
    }
    else
    {
        return m_member_a;
    }
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

    // These checks should never trigger.
    // If they do, then something is seriously wrong with the client.
    // TODO: Should we delete the Trade object if these checks fail?
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
    // TODO: Should we delete the Trade object if these checks fail?
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

void updateTrade(Entity& src, const TradeUpdateInfo& info)
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

    trade_src.m_accepted = info.m_accepted;
    trade_src.m_influence = info.m_influence;
    // TODO: Inspirations and enhancements.

    sendTradeUpdate(*tgt, trade_src, trade_tgt);
    qCDebug(logSlashCommand) << src.name() << "updated a trade with" << tgt->name(); // TODO: Trade debug

    if (trade_src.m_accepted && trade_tgt.m_accepted)
    {
        finishTrade(src, *tgt);
    }
}

void discardTrade(Entity& ent)
{
    ent.m_trade.reset();
}

//! @}
