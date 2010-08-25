/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once

#include <map>
#include <vector>
#include "EntityStorage.h"
#include "EventProcessor.h"

class SceneRequest;
class EntitiesRequest;

class MapInstance : public EventProcessor
{
    std::string m_name;
public:
    EntityManager	m_entities;
                    MapInstance(const std::string &name);
    void			dispatch(SEGSEvent *ev);
    SEGSEvent * 	dispatch_sync(SEGSEvent *ev);
protected:
    void            on_scene_request(SceneRequest *ev);
    void            on_entities_request(EntitiesRequest *ev);
    void            on_timeout(TimerEvent *ev);
};
