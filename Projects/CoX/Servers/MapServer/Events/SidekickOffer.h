/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "Logging.h"

class SidekickOffer final : public GameCommand
{
public:
    uint32_t m_db_id;
            SidekickOffer(uint32_t &db_id) : GameCommand(MapEventTypes::evSidekickOffer),
                m_db_id(db_id)
            {
            }
    void    serializeto(BitStream &bs) const override 
            {
                bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 28
                bs.StoreBits(32,m_db_id);
                qCDebug(logTeams) << "Sidekick Offer db_id:" << m_db_id;
            }
    void    serializefrom(BitStream &src);
};
