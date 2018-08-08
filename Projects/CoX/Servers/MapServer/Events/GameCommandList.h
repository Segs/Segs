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
class GameCommand
{
    const uint32_t    m_type;
public:
                    GameCommand(uint32_t type) : m_type(type) {}
virtual             ~GameCommand() = default;
        uint32_t    type() const {return m_type;}
virtual void        serializeto(BitStream &bs) const = 0;
};

#include "Events/ChatMessage.h"
#include "Events/StandardDialogCmd.h"
#include "Events/InfoMessageCmd.h"
#include "Events/FloatingDamage.h"
