/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUD_Link.h"
#include "GameEventFactory.h"

struct GameLink : public CRUDLink
{
    GameLink(EventSrc *tgt,EventSrc *net_layer)
    {
        m_net_layer = net_layer;
        m_target = tgt;
        assert(tgt);
    }
    // CRUDLink interface
protected:
    CRUD_EventFactory &factory() override
    {
        return m_factory;
    }
    static GameEventFactory m_factory;

};
