/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "FloatingInfoStyles.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

// [[ev_def:type]]
class FloatingInfo final : public GameCommand
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
                FloatingInfo(uint32_t tgt_idx, QString msg, FloatingInfoStyle style, float delay) : GameCommand(MapEventTypes::evFloatingInfo),
                    m_tgt_idx(tgt_idx),
                    m_msg(msg),
                    m_style(style),
                    m_delay(delay)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient); // 56

                    bs.StorePackedBits(1, m_tgt_idx);
                    bs.StoreString(m_msg);
                    bs.StorePackedBits(2, m_style);
                    bs.StoreFloat(m_delay);
                }
        void    serializefrom(BitStream &src);
};
