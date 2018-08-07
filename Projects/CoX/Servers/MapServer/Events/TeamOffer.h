/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "Logging.h"

#include "MapEvents.h"
#include "MapLink.h"

enum TeamOfferType :  uint8_t
{
    NoMission       = 0,
    WithMission     = 1,
    LeaveMission    = 2,
};

// [[ev_def:type]]
class TeamOffer final : public GameCommand
{
public:
    // [[ev_def:field]]
    uint32_t m_db_id;
    // [[ev_def:field]]
    QString m_name;
    // [[ev_def:field]]
    TeamOfferType m_offer_type;
    TeamOffer(uint32_t &db_id, QString &name, TeamOfferType &type) : GameCommand(MapEventTypes::evTeamOffer),
        m_db_id(db_id),
        m_name(name),
        m_offer_type(type)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 24
        bs.StoreBits(32,m_db_id);   // team offeree db_id
        bs.StoreString(m_name);     // team offerer name
        bs.StoreBits(2,m_offer_type);     // team with mission?
    }
    void    serializefrom(BitStream &src);
};
