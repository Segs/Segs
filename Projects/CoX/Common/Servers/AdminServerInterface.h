/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Server.h"

#include <ace/INET_Addr.h>
#include <list>

struct AuthAccountData;

class IMapServer;
class IGameServer;
class IAdminServer : public Server
{
public:
virtual bool            Login(const AuthAccountData &client,const ACE_INET_Addr &client_addr)=0;
virtual bool            ValidPassword(const AuthAccountData &client, const char *password)=0;

virtual bool            fill_account_info(AuthAccountData &client)=0;
virtual int             SaveAccount(const char *username, const char *password)=0;
};

class AdminServerInterface : public Server
{
public:
                        AdminServerInterface(IAdminServer *srv);
                        ~AdminServerInterface(void);

        bool            ReadConfig();
        bool            Run(void);
        bool            ShutDown(const QString &reason);

        int             GetBlockedIpList(std::list<int> &addreses);
        bool            FillClientInfo(AuthAccountData &);
        bool            Login(AuthAccountData &client,const ACE_INET_Addr &client_addr);
        int             SaveAccount(const char *username, const char *password);
        bool            ValidPassword(const AuthAccountData &client, const char *password);
protected:
        IAdminServer *  m_server;
};
