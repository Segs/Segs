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
class World;
class MapInstance : public EventProcessor
{
typedef std::vector<MapClient *> vClients;
        std::string     m_name;
        SEGSTimer *     m_world_update_timer;
        SEGSTimer *     m_resend_timer;
        vClients        m_clients;
        vClients        m_queued_clients;
        World *         m_world;
public:
        EntityManager   m_entities;

                        MapInstance(const std::string &name);
virtual                 ~MapInstance();
        void            dispatch(SEGSEvent *ev);
        SEGSEvent *     dispatch_sync(SEGSEvent *ev);

        void            enqueue_client(MapClient *clnt);
        void            start();
protected:
        void            on_scene_request(SceneRequest *ev);
        void            on_entities_request(EntitiesRequest *ev);
        void            on_timeout(TimerEvent *ev);
        void            on_combine_boosts(CombineRequest *);
        void            on_input_state(InputState *st);

        void            sendState();
};
