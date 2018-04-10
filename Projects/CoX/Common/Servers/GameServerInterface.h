/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <string>
#include <stdint.h>

#include "Server.h"
#include "ServerHandle.h"
class EventProcessor;
class IMapServer;
// this is a proxy for calling GameServer services
class IGameServer : public Server
{
public:
virtual const ACE_INET_Addr &   getAddress()=0;
virtual QString                 getName(void)=0;
virtual uint8_t                 getId(void)=0;
virtual uint16_t                getCurrentPlayers(void)=0;
virtual uint16_t                getMaxPlayers()=0;
virtual EventProcessor *        event_target()=0;
virtual int                     getMaxCharacterSlots() const = 0;
};

class GameServerInterface : public IGameServer
{
public:
                                GameServerInterface(IGameServer *mi) : m_instance(mi){}
                                ~GameServerInterface(void){}
        //uint32_t GetClientCookie(const ACE_INET_Addr &client_addr);
        bool                    ReadConfig();
        bool                    Run(void);
        bool                    ShutDown(const QString &reason);

        const ACE_INET_Addr &   getAddress();

        uint8_t                 getId();
        uint16_t                getCurrentPlayers();
        uint16_t                getMaxPlayers();
        QString                 getName();

        EventProcessor *        event_target(); // this is the main communication point for the Game Server instance
        int                     getMaxCharacterSlots() const ;

protected:
        IGameServer *           m_instance;
};
