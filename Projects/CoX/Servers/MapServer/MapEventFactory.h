/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUD_Events.h"

typedef CRUDLink_Event MapLinkEvent;

/////////////////////////////////////////////////////////////////////
class MapEventFactory : public CRUD_EventFactory
{
public:
    MapLinkEvent *EventFromStream(BitStream &bs) override;
    static MapLinkEvent *CommandEventFromStream(BitStream &bs);
};



