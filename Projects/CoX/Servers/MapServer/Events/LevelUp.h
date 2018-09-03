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
namespace SEGSEvents
{
// [[ev_def:type]]
class LevelUp final : public GameCommandEvent
{
public:
                LevelUp() : GameCommandEvent(evLevelUp)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 56

                    // nothing to send
                }
        EVENT_IMPL(LevelUp)
};
} // end of SEGSEvents namespace
