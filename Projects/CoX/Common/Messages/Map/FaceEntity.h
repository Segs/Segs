/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommandList.h"
#include "MapEvents.h"
#include "GameData/Entity.h"

#include <QtCore/QString>

namespace SEGSEvents
{
    // [[ev_def:type]]
    class FaceEntity : public GameCommandEvent
    {
    public:
        explicit FaceEntity() : GameCommandEvent(MapEventTypes::evFaceEntity) {}
        FaceEntity(uint32_t target) : GameCommandEvent(MapEventTypes::evFaceEntity),
            m_target(target)
        {}

        // SerializableEvent interface
        void serializefrom(BitStream &/*src*/) override
        {}
        void serializeto(BitStream &bs) const override {
            bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient);
            bs.StorePackedBits(3, m_target);
        }
        EVENT_IMPL(FaceEntity)

    protected:
        //  [[ev_def:field]]
        uint32_t m_target;
    };
}
