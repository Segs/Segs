/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once
#include "Common/CRUDP_Protocol/CRUD_Link.h"
#include "GameEventFactory.h"

struct GameLink : public CRUDLink
{
    GameLink(EventProcessor *tgt,EventProcessor *net_layer) : CRUDLink() {
        m_net_layer = net_layer;
        m_target = tgt;
        assert(tgt);
    }
    CharacterClient * client_data() {return static_cast<CharacterClient *>(m_link_data);}


    // CRUDLink interface
protected:
    CRUD_EventFactory &factory() override
    {
        return m_factory;
    }
    static GameEventFactory m_factory;

};
