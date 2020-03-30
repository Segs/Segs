/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUD_Events.h"

namespace SEGSEvents
{
using MapLinkEvent = CRUDLink_Event;
}

/////////////////////////////////////////////////////////////////////
class MapEventFactory : public CRUD_EventFactory
{
public:
    SEGSEvents::MapLinkEvent *EventFromStream(BitStream &bs) override;
    static SEGSEvents::MapLinkEvent *CommandEventFromStream(BitStream &bs);
};



