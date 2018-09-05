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
class SendStance final : public GameCommand
{
public:
    // [[ev_def:field]]
    PowerStance m_stance;
                SendStance(PowerStance pow_stance) : GameCommand(MapEventTypes::evSendStance),
                    m_stance(pow_stance)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient); // 57

                    bs.StoreBits(1, m_stance.has_stance);
                    if(!m_stance.has_stance)
                        return;
                    bs.StorePackedBits(4, m_stance.pset_idx);
                    bs.StorePackedBits(4, m_stance.pow_idx);
                }
        void    serializefrom(BitStream &src);
};
