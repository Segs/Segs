/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>


class Entity;


class TradeMember {
public:
    explicit TradeMember(const Entity& ent);

    uint32_t m_db_id     = 0;
    bool     m_accepted  = false;
    uint32_t m_influence = 0;
    // TODO: Inspirations and enhancements.
};


class Trade {
public:
    Trade(const Entity& ent_a, const Entity& ent_b);
    TradeMember& getMember(Entity& ent);
    TradeMember& getOtherMember(Entity& ent);

    TradeMember m_member_a;
    TradeMember m_member_b;
    bool m_invite_accepted  = false;
};


struct TradeUpdateInfo {
    uint32_t m_db_id        = 0;        // Database id of other player.
    bool     m_accepted     = false;
    uint32_t m_influence    = 0;
    // TODO: Inspirations and enhancements.
};


void requestTrade(Entity& src, Entity& tgt);
void acceptTrade(Entity& src, Entity& tgt);
void declineTrade(Entity& src, Entity& tgt);
void cancelTrade(Entity& src);
void updateTrade(Entity& src, const TradeUpdateInfo& info);
void discardTrade(Entity& src);
