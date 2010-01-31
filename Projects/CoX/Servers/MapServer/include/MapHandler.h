/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <map>
#include "InternalEvents.h"
#include "EventProcessor.h"
#include "ClientManager.h"
#include "MapEvents.h"

class CoXMap;
class MapClient;
class MapServer;
class IClient;
class MapCommHandler : public EventProcessor
{
public:
	void        set_server(MapServer *s) {m_server=s;}
                MapCommHandler();
protected:
	void        dispatch(SEGSEvent *ev);
	SEGSEvent * dispatch_sync( SEGSEvent *ev );
    //////////////////////////////////////////////////////////////////////////
    //
    void        on_disconnect(DisconnectRequest<MapLinkEvent> *ev);
    void        on_idle(IdleEvent<MapLinkEvent> *ev);
    void        on_connection_request(ConnectRequest<MapLinkEvent> *ev);
    void        on_shortcuts_request(ShortcutsRequest *ev);
    void        on_scene_request(SceneRequest *ev);
    void        on_entities_request(EntitiesRequest *ev);
    void        on_timeout(TimerEvent *ev);
	//////////////////////////////////////////////////////////////////////////
	// Server <-> Server events
	void        on_expect_client(ExpectMapClient *ev);
    void        on_create_map_entity(NewEntity *ent);
    ClientStore<MapClient> m_clients;
	MapServer *m_server;
    std::map<int,CoXMap *> m_handled_worlds;//! the worlds run by this server
};
