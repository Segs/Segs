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
class FloatingDamage final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    int m_damage_source;
    // [[ev_def:field]]
    int m_damage_target;
    // [[ev_def:field]]
    int m_amount; // should be float?

    explicit FloatingDamage() : GameCommandEvent(evFloatingDamage) {}
    FloatingDamage(int source,int target,int amount) : GameCommandEvent(evFloatingDamage),
        m_damage_source(source),
        m_damage_target(target),
        m_amount(amount)
    {
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-evFirstServerToClient); // packet 21

        bs.StorePackedBits(1,m_damage_source);
        bs.StorePackedBits(1,m_damage_target);
        bs.StorePackedBits(1,m_amount);
    }
    EVENT_IMPL(FloatingDamage)
};

} //end of SEGSEvents namespace
