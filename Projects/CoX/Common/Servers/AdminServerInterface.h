/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <list>
#include "ServerHandle.h"
#include "RoamingServer.h"
/************************************************************************/
/* Design consideration:                                                */
/* Current implementation gets knowledge it shouldn't possess,          */
/* AdminServerInterface::GetClientByLogin returns all details known     */
/* about given client.                                                  */
/************************************************************************/

// this is interface for calling AdminServer services, currently it's an Interface pattern
// in future it can be a Remote Proxy

class AccountInfo;

class IMapServer;
class IGameServer;
class IAdminServer : public RoamingServer
{
        typedef ServerHandle<IGameServer> hGameServer;
public:
virtual bool            Login(const AccountInfo &client,const ACE_INET_Addr &client_addr)=0;
virtual bool            ValidPassword(const AccountInfo &client, const char *password)=0;

virtual bool            fill_account_info(AccountInfo &client)=0;
virtual int             SaveAccount(const char *username, const char *password)=0;
virtual int             RemoveAccount(AccountInfo &client)=0;
};

class AdminServerInterface : public Server
{
        typedef ServerHandle<IGameServer> hGameServer;
public:
                        AdminServerInterface(IAdminServer *srv);
                        ~AdminServerInterface(void);

        bool            ReadConfig();
        bool            Run(void);
        bool            ShutDown(const QString &reason);

        int             GetBlockedIpList(std::list<int> &addreses);
        bool            FillClientInfo(AccountInfo &);
        bool            Login(AccountInfo &client,const ACE_INET_Addr &client_addr);
        int             SaveAccount(const char *username, const char *password);
        bool            ValidPassword(const AccountInfo &client, const char *password);
protected:
        IAdminServer *  m_server;
};
