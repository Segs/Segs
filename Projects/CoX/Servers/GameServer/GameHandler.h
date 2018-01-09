#pragma once
#include "ClientManager.h"
#include "GameLink.h"
#include "GameEvents.h"
#include "EventProcessor.h"
#include "InternalEvents.h"

#include <map>
#include <unordered_set>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
class CharacterClient;
class GameServer;
class GameHandler : public EventProcessor
{
    typedef std::unordered_set<uint32_t> sIds;
    // function that send messages into the link
    // incoming event handlers
    class SEGSTimer *     m_link_checker=nullptr;
public:
    void        set_server(GameServer *s) {m_server=s;}
    void        start();
protected:
        void    dispatch(SEGSEvent *ev);
    SEGSEvent * dispatchSync( SEGSEvent *ev );

    //////////////////////////////////////////////////////////////////////////
    // Link events
    void        on_idle(IdleEvent *ev);
    void        on_link_lost(SEGSEvent *ev);
    void        on_disconnect(DisconnectRequest *ev);
    void        on_connection_request(ConnectRequest *ev);

    void        on_update_server(UpdateServer *ev);
    void        on_update_character(UpdateCharacter *ev);
    void        on_delete_character(DeleteCharacter *ev);
    void        on_map_req(MapServerAddrRequest *ev);
    void        on_unknown_link_event(GameUnknownRequest *ev);
    //////////////////////////////////////////////////////////////////////////
    // Server <-> Server events
    void        on_expect_client(ExpectClient *ev);     // from AuthServer
    void        on_client_expected(ClientExpected *ev); // from MapServer

    //////////////////////////////////////////////////////////////////////////
    // Internal events
    void        on_check_links();
    void        on_timeout(TimerEvent *ev);

    // synchronous event
    SEGSEvent * on_connection_query(ClientConnectionQuery *ev);

    //////////////////////////////////////////////////////////////////////////
    void        checkClientConnection(uint64_t id);
    bool        isClientConnected(uint64_t id);
    void        disconnectClient( AccountInfo & cl );
    sIds        waiting_for_client; // this hash_set holds all client cookies we wait for
    ClientStore<CharacterClient>    m_clients;
    GameServer *m_server;

};
typedef ACE_Singleton<GameHandler,ACE_Thread_Mutex> GameHandlerG;
