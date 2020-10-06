/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"

#include "MapEventTypes.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class ChangeTitle final : public GameCommandEvent
{
public:
        // [[ev_def:field]]
        bool m_select_origin;

        ChangeTitle(bool select_origin=false) : GameCommandEvent(evChangeTitle),
                    m_select_origin(select_origin)
        {
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // 56
            bs.StoreBits(1, m_select_origin);
        }
        EVENT_IMPL(ChangeTitle)
};

} // end of SEGSEvent namespace
