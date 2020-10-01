/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEvents.h"

namespace SEGSEvents
{

// [[ev_def:type]]
class FaceEntity : public GameCommandEvent
{
public:
    explicit FaceEntity() : GameCommandEvent(evFaceEntity) {}
    FaceEntity(uint32_t target) : GameCommandEvent(evFaceEntity),
        m_target(target)
    {}

    // SerializableEvent interface
    void serializefrom(BitStream &/*src*/) override
    {}
    void serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1,type()-evFirstServerToClient);
        bs.StorePackedBits(3, m_target);
    }
    EVENT_IMPL(FaceEntity)

protected:
    //  [[ev_def:field]]
    uint32_t m_target;
};

} //end of SEGSEvents namespace
