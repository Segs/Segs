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
class FloatingDamage final : public GameCommand
{
public:
    // [[ev_def:field]]
    int whos_fault_was_it;
    // [[ev_def:field]]
    int who_was_damaged;
    // [[ev_def:field]]
    int damage_amount; // should be float?
    FloatingDamage(int source,int target,int amount) : GameCommand(MapEventTypes::evFloatingDamage),
        whos_fault_was_it(source),
        who_was_damaged(target),
        damage_amount(amount)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);

        bs.StorePackedBits(1,whos_fault_was_it);
        bs.StorePackedBits(1,who_was_damaged);
        bs.StorePackedBits(1,damage_amount);
    }
    void    serializefrom(BitStream &src);
};
