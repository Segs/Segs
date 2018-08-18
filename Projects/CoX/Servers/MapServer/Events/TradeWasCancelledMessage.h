/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
//#include <glm/vec3.hpp>

class TradeWasCancelledMessage : public MapLinkEvent
{
public:
    TradeWasCancelledMessage();
    virtual ~TradeWasCancelledMessage() override = default;

    // SerializableEvent interface
    virtual void serializefrom(BitStream &bs) override;
    virtual void serializeto(BitStream &) const override;

    int m_reason = 0; // 0 - target entity no longer exists, 1 - user selects cancel button.
};
