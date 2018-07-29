/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"
#include "ClientStates.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

class SetClientState final : public GameCommand
{
public:
    ClientStates    m_new_state;
                SetClientState(ClientStates new_state) : GameCommand(MapEventTypes::evSetClientState),
                    m_new_state(new_state)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient); // 18

                    bs.StorePackedBits(1, uint32_t(m_new_state));
                }
        void    serializefrom(BitStream &src);
};
