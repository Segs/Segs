/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

class FloatingDamage final : public GameCommand
{
public:
    int whos_fault_was_it;
    int who_was_damaged;
    int damage_amount;
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
