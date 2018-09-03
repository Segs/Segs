/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Events/MapEventTypes.h"
#include "GameCommandList.h"
#include "ClientStates.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
namespace SEGSEvents
{

// [[ev_def:type]]
class SetClientState final : public GameCommandEvent
{
public:
        // [[ev_def:field]]
        ClientStates    m_new_state;
                        SetClientState(ClientStates new_state=SIMPLE) : GameCommandEvent(evSetClientState), m_new_state(new_state)
                        {
                        }

        void            serializeto(BitStream &bs) const override
                        {
                            bs.StorePackedBits(1, type() - evFirstServerToClient); // 18
                            bs.StorePackedBits(1, uint32_t(m_new_state));
                        }

                        EVENT_IMPL(SetClientState)
};
} // end of namespace SEGSEvents
