/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "MapEvents.h"
#include "MapLink.h"
#include "NetStructures/Trade.h"

#include <QtCore/QString>
//#include <glm/vec3.hpp>

class TradeWasUpdatedMessage : public MapLinkEvent
{
public:
    TradeWasUpdatedMessage();
    virtual ~TradeWasUpdatedMessage() override = default;

    // SerializableEvent interface
    virtual void serializefrom(BitStream &bs) override;
    virtual void serializeto(BitStream &) const override;

    TradeInfo m_info;
};
