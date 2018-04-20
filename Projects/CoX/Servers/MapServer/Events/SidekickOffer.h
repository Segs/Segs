/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
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
