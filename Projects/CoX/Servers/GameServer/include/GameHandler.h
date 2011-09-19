#pragma once
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Addr.h>
#include <map>
#include "ClientManager.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "EventProcessor.h"
#include "InternalEvents.h"

class CharacterClient;
class GameServer;
class GameHandler : public EventProcessor
{
    // function that send messages into the link
    // incoming event handlers
public:
    void        set_server(GameServer *s) {m_server=s;}
protected:
        void        dispatch(SEGSEvent *ev);
    SEGSEvent * dispatch_sync( SEGSEvent *ev );

    //////////////////////////////////////////////////////////////////////////
    // Link events
    void        on_idle(IdleEvent *ev);
    void        on_disconnect(DisconnectRequest *ev);
    void        on_connection_request(ConnectRequest *ev);

    void        on_update_server(UpdateServer *ev);
    void        on_update_character(UpdateCharacter *ev);
    void        on_delete_character(DeleteCharacter *ev);
    void        on_map_req(MapServerAddrRequest *ev);
    void        on_unknown_link_event(GameUnknownRequest *ev);
    //////////////////////////////////////////////////////////////////////////
    // Server <-> Server events
    void        on_expect_client(ExpectClient *ev);		// from AuthServer
        void        on_client_expected(ClientExpected *ev); // from MapServer

    // synchronous event
    SEGSEvent * on_connection_query(ClientConnectionQuery *ev);

    //////////////////////////////////////////////////////////////////////////
    void        checkClientConnection(u64 id);
    bool        isClientConnected(u64 id);
    void disconnectClient( AccountInfo & cl );
    hash_set<u32>           waiting_for_client; // this hash_set holds all client cookies we wait for
    ClientStore<CharacterClient> m_clients;
    GameServer *m_server;

};
typedef ACE_Singleton<GameHandler,ACE_Thread_Mutex> GameHandlerG;
