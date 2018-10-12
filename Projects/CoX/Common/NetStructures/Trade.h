/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <vector>
#include <stdint.h>


class Entity;


struct TradeInspiration {
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
struct TradeInfo {
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


class TradeMember {
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


class Trade {
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


void requestTrade(Entity& src, Entity& tgt);
void acceptTrade(Entity& src, Entity& tgt);
void declineTrade(Entity& src, Entity& tgt);
void cancelTrade(Entity& src);
void updateTrade(Entity& src, const TradeInfo& info);
void discardTrade(Entity& src);
