#pragma once
#include "CRUDP_Protocol/CRUD_Events.h"
#include "Common/CRUDP_Protocol/CRUD_Link.h"
typedef CRUDLink_Event GameLinkEvent;
class CharacterClient;

// TODO: Is GameEventFactory a misleading name, since this class is not a factory for 'all' game events ?
class GameEventFactory : public CRUD_EventFactory
{
public:
    typedef CharacterClient tClientData ; //!< typedef used by CRUDLink to store per-connection data
    GameLinkEvent *EventFromStream(BitStream &bs,bool followe=false) override;
};
