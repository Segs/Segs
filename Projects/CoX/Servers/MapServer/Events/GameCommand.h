/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "MapEventTypes.h"
#include "MapLink.h"

class QString;
//TODO: those must support chaining
namespace SEGSEvents
{
class GameCommandEvent : public Event
{
public:
                    GameCommandEvent(uint32_t type) : Event(type) {}
virtual             ~GameCommandEvent() = default;
virtual void        serializeto(BitStream &bs) const = 0;
};
} // end of SEGSEvents namespace
