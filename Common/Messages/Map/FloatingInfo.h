/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "FloatingInfoStyles.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

namespace SEGSEvents
{
// [[ev_def:type]]
class FloatingInfo final : public GameCommandEvent
{
public:
                        // [[ev_def:field]]
    uint32_t            m_tgt_idx;
                        // [[ev_def:field]]
    QString             m_msg;
                        // [[ev_def:field]]
    FloatingInfoStyle   m_style;
                        // [[ev_def:field]]
    float               m_delay;
    explicit            FloatingInfo() : GameCommandEvent(evFloatingInfo) {}
                        FloatingInfo(uint32_t tgt_idx, QString msg, FloatingInfoStyle style, float delay) : GameCommandEvent(evFloatingInfo),
                            m_tgt_idx(tgt_idx),
                            m_msg(msg),
                            m_style(style),
                            m_delay(delay)
                        {
                        }

        void            serializeto(BitStream &bs) const override
                        {
                            bs.StorePackedBits(1, type()-evFirstServerToClient); // 56

                            bs.StorePackedBits(1, m_tgt_idx);
                            bs.StoreString(m_msg);
                            bs.StorePackedBits(2, m_style);
                            bs.StoreFloat(m_delay);
                        }
                        EVENT_IMPL(FloatingInfo)
};
} // end of SEGSEvents namespace
