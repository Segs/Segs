/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/SEGSEvent.h"

class QString;
class BitStream;

namespace SEGSEvents
{

class GameCommandEvent : public Event
{
public:
                    GameCommandEvent(uint32_t type) : Event(type) {}
virtual void        serializeto(BitStream &bs) const = 0;
virtual void        serializefrom(BitStream &) { assert(false); }
};

} // end of SEGSEvents namespace
