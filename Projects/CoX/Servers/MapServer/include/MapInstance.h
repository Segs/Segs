/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */

#pragma once

#include <map>
#include <vector>
#include "EntityStorage.h"
#include "EventProcessor.h"

class SceneRequest;
class CombineRequest;
class EntitiesRequest;
class SEGSTimer;
class InputState;
class MapInstance : public EventProcessor
{
    std::string     m_name;
    SEGSTimer *     m_world_update_timer;
    std::vector<MapClient *> m_clients;
    std::vector<MapClient *> m_queued_clients;
public:
    EntityManager   m_entities;
                    MapInstance(const std::string &name);
    void            dispatch(SEGSEvent *ev);
    SEGSEvent *     dispatch_sync(SEGSEvent *ev);

    void            enqueue_client(MapClient *clnt);

protected:
    void            on_scene_request(SceneRequest *ev);
    void            on_entities_request(EntitiesRequest *ev);
    void            on_timeout(TimerEvent *ev);
    void            on_combine_boosts(CombineRequest *);
    void            on_input_state(InputState *st);
};
