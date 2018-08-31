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
#include "NetStructures/Entity.h"

#include <QtCore/QString>

class FaceEntity : public GameCommand
{
public:
    FaceEntity(uint32_t target) : GameCommand(MapEventTypes::evFaceEntity),
        m_target(target) 
    {}
    
    // SerializableEvent interface
    void serializefrom(BitStream &src);
    void serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
        bs.StorePackedBits(3, m_target);
    };

protected:
    uint32_t m_target;
};