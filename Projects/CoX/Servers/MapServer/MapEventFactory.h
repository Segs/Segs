#pragma once
#include "CRUD_Events.h"
#include "CRUD_Link.h"

typedef CRUDLink_Event MapLinkEvent;
class MapClient;
/////////////////////////////////////////////////////////////////////
class MapEventFactory : public CRUD_EventFactory
{
public:
    static MapLinkEvent *EventFromStream(BitStream &bs, bool follower=false);
    typedef MapClient tClientData;
};
typedef CRUDLink<MapEventFactory> MapLink;

