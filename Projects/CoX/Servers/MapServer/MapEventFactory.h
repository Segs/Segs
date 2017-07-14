#pragma once
#include "Common/CRUDP_Protocol/CRUD_Events.h"

typedef CRUDLink_Event MapLinkEvent;
class MapClient;
/////////////////////////////////////////////////////////////////////
class MapEventFactory : public CRUD_EventFactory
{
public:
    MapLinkEvent *EventFromStream(BitStream &bs, bool follower=false) override;
};



