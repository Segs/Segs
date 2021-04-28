/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEventTypes.h"
#include "GameCommand.h"
#include "Components/BitStream.h"

enum class TeamOfferType :  uint8_t
{
    NoMission       = 0,
    WithMission     = 1,
    LeaveMission    = 2,
};

namespace SEGSEvents
{

// [[ev_def:type]]
class TeamOffer final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_db_id;
    // [[ev_def:field]]
    QString m_name;
    // [[ev_def:field]]
    TeamOfferType m_offer_type;

explicit    TeamOffer() : GameCommandEvent(MapEventTypes::evTeamOffer) {}
            TeamOffer(uint32_t &db_id, QString &name, TeamOfferType &type) : GameCommandEvent(MapEventTypes::evTeamOffer),
                m_db_id(db_id),
                m_name(name),
                m_offer_type(type)
            {
            }
    void    serializeto(BitStream &bs) const override
            {
                bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // packet 24
                bs.StoreBits(32, m_db_id);   // team offeree db_id
                bs.StoreString(m_name);     // team offerer name
                bs.StoreBits(2, uint8_t(m_offer_type));     // team with mission?
            }
            EVENT_IMPL(TeamOffer)
};

} // end of SEGSEvents namespace

