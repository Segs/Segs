/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUDP_Protocol/CRUD_Events.h"
#include "Common/CRUDP_Protocol/CRUD_Link.h"
typedef CRUDLink_Event GameLinkEvent;
class CharacterClient;

// TODO: Is GameEventFactory a misleading name, since this class is not a factory for 'all' game events ?
class GameEventFactory final : public CRUD_EventFactory
{
public:
    typedef CharacterClient tClientData ; //!< typedef used by CRUDLink to store per-connection data
    GameLinkEvent *EventFromStream(BitStream &bs) override;
};
