/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <string>
#include "Base.h"
#include "RoamingServer.h"
#include "ServerHandle.h"
class EventProcessor;
class IMapServer;
// this is a proxy for calling GameServer services
class IGameServer : public RoamingServer
{
public:
virtual const ACE_INET_Addr &	getAddress()=0;
virtual std::string				getName(void)=0;
virtual uint8_t					getId(void)=0;
virtual uint16_t				getCurrentPlayers(void)=0;
virtual uint16_t				getMaxPlayers()=0;
virtual uint8_t					getUnkn1(void)=0;
virtual uint8_t					getUnkn2(void)=0;
virtual int					getAccessKeyForServer(const ServerHandle<IMapServer> &h_map)=0;
virtual bool					isMapServerReady(const ServerHandle<IMapServer> &h_map)=0;
virtual EventProcessor *        event_target()=0;
};

class GameServerInterface : public IGameServer
{
public:
    GameServerInterface(IGameServer *mi) : m_instance(mi){};
    ~GameServerInterface(void){};
    //uint32_t GetClientCookie(const ACE_INET_Addr &client_addr);
    bool					ReadConfig(const std::string &name); //! later name will be used to read GameServer specific configuration
    bool					Run(void);
    bool					ShutDown(const std::string &reason);

    int						getAccessKeyForServer(const ServerHandle<IMapServer> &h_map);
    bool					isMapServerReady(const ServerHandle<IMapServer> &h_map);

    bool					isLocal(){return true;} //! this method returns true if this interface is a local ( same process )
    bool					Online();
    const ACE_INET_Addr &	getAddress();

    uint8_t						getId();
    uint16_t					getCurrentPlayers();
    uint16_t					getMaxPlayers();
    uint8_t						getUnkn1();
    uint8_t						getUnkn2();
    std::string				getName();

    EventProcessor *        event_target(); // this is the main communication point for the Game Server instance

protected:
    IGameServer *m_instance;
};
