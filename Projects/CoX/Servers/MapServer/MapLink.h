/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
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
    MapClient * client_data() {return static_cast<MapClient *>(m_link_data);}


    // CRUDLink interface
protected:
    CRUD_EventFactory &factory() override
    {
        return m_factory;
    }
    static MapEventFactory m_factory;

};
