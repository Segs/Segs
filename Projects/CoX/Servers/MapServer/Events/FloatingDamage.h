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

class FloatingDamage final : public GameCommand
{
public:
    int m_source;
    int m_target;
    int m_dmg_amount;
    FloatingDamage(int source,int target,int amount) : GameCommand(MapEventTypes::evFloatingDamage),
        m_source(source),
        m_target(target),
        m_dmg_amount(amount)
    {
    }
    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);

        bs.StorePackedBits(1,m_source);
        bs.StorePackedBits(1,m_target);
        bs.StorePackedBits(1,m_dmg_amount);
    }
    void    serializefrom(BitStream &src);
};
