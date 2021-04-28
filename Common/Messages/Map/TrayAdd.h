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

#include <QtCore/QString>
namespace SEGSEvents
{
// [[ev_def:type]]
class TrayAdd final : public GameCommandEvent
{
public:
                        // [[ev_def:field]]
    uint32_t            m_pset_idx;
                        // [[ev_def:field]]
    uint32_t            m_pow_idx;
    explicit            TrayAdd() : GameCommandEvent(evTrayAdd) {}
                        TrayAdd(uint32_t pset_idx, uint32_t pow_idx) : GameCommandEvent(evTrayAdd),
                            m_pset_idx(pset_idx),
                            m_pow_idx(pow_idx)
                        {
                        }

    void                serializeto(BitStream &bs) const override {
                            bs.StorePackedBits(1, type()-evFirstServerToClient); // 56

                            bs.StorePackedBits(1, m_pset_idx);
                            bs.StorePackedBits(1, m_pow_idx);
                        }
                        EVENT_IMPL(TrayAdd)
};
} // end of SEGSEvents namespace
