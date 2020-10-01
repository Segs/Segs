/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <vector>
#include <stdint.h>

class Entity;

enum class TradeSystemMessages
{
    SUCCESS,
    GENERIC_FAILURE=1,
    NOT_ENOUGH_ROOM_INFLUENCE,
    NOT_ENOUGH_ROOM_ENHANCEMENTS,
    NOT_ENOUGH_ROOM_INSPIRATIONS,
    SRC_ALREADY_IN_TRADE,
    TGT_ALREADY_IN_TRADE,
    SRC_CONSIDERING_ANOTHER_TRADE,
    TGT_CONSIDERING_ANOTHER_TRADE,
    SEND_TRADE_OFFER,
    TRADE_REQUEST_IS_VALID,
    HAS_SENT_NO_TRADE,
    TGT_RECV_NO_TRADE,
    SRC_RECV_NO_TRADE,
    NOT_CONSIDERING_TRADE,
    ACCEPTED_TRADE,
    DECLINED_TRADE,
};

struct TradeInspiration
{
    TradeInspiration() = default;
    TradeInspiration(uint32_t col, uint32_t row);
    bool operator==(const TradeInspiration& other) const;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_col);
        ar(m_row);
    }

    uint32_t m_col = 0;
    uint32_t m_row = 0;
};


// Trade information received from the client.
struct TradeInfo
{
    bool isEqualContent(const TradeInfo& info) const;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_db_id);
        ar(m_accepted);
        ar(m_influence);
        ar(m_enhancements);
        ar(m_inspirations);
    }

    uint32_t                        m_db_id         = 0;    // Database id of other player.
    bool                            m_accepted      = false;
    uint32_t                        m_influence     = 0;
    std::vector<uint32_t>           m_enhancements;         // Index into enhancement list of player.
    std::vector<TradeInspiration>   m_inspirations;         // column, row into inspiration grid of player.
};


class TradeMember
{
public:
    TradeMember();
    explicit TradeMember(const Entity& ent);

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_db_id);
        ar(m_info);
    }

    uint32_t  m_db_id = 0; // Player database id.
    TradeInfo m_info;
};


class Trade
{
public:
    Trade(const Entity& ent_a, const Entity& ent_b);
    TradeMember& getMember(const Entity& ent);
    const TradeMember& getMember(const Entity& ent) const;
    TradeMember& getOtherMember(const Entity& ent);
    const TradeMember& getOtherMember(const Entity& ent) const;
    bool isAccepted() const;

    TradeMember m_member_a;
    TradeMember m_member_b;
    bool m_invite_accepted  = false;
};


TradeSystemMessages requestTrade(Entity& src, Entity& tgt);
TradeSystemMessages acceptTrade(Entity& src, Entity& tgt);
TradeSystemMessages declineTrade(Entity& src, Entity& tgt);
TradeSystemMessages updateTrade(Entity& src, Entity &tgt, const TradeInfo& info);
void discardTrade(Entity& src);
void finishTrade(Entity& src, Entity& tgt);
