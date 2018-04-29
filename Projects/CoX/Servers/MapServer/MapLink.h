/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUD_Link.h"
#include "MapEventFactory.h"
struct MapLink : public CRUDLink
{
    MapLink(EventProcessor *tgt,EventProcessor *linktgt) : CRUDLink() {
        m_net_layer = linktgt;
        m_target = tgt;
        assert(tgt);
    }
    // CRUDLink interface
protected:
    CRUD_EventFactory &factory() override
    {
        return m_factory;
    }
    static MapEventFactory m_factory;

};
