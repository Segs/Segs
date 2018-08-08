/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

// [[ev_def:type]]
class TrayAdd final : public GameCommand
{
public:
    // [[ev_def:field]]
    uint32_t            m_pset_idx;
    // [[ev_def:field]]
    uint32_t            m_pow_idx;
                TrayAdd(uint32_t pset_idx, uint32_t pow_idx) : GameCommand(MapEventTypes::evTrayAdd),
                    m_pset_idx(pset_idx),
                    m_pow_idx(pow_idx)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient); // 56

                    bs.StorePackedBits(1, m_pset_idx);
                    bs.StorePackedBits(1, m_pow_idx);
                }
        void    serializefrom(BitStream &src);
};
