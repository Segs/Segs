/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once

#include <list>
#include <string>

// ACE includes
#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>

// segs includes
#include "Server.h"
#include "ServerHandle.h"
#include "AdminServerInterface.h"

class AdminDatabase;
class CharacterDatabase;
class IGameServer;
class IMapServer;
class AccountInfo;
//! The AdminServer class handles administrative functions such as account saving, account banning, etcetera.
class _AdminServer : public IAdminServer
{
public:

    // Constructor/Destructor
                                _AdminServer(void);
                                ~_AdminServer(void);

    // Client handling related interface
    int                         GetBlockedIpList(std::list<int> &); // Called from auth server during user authentication, might be useful for automatical firewall rules update

    bool                        Logout(const AccountInfo &client) const; // Client logout
    bool                        Login(const AccountInfo &client,const ACE_INET_Addr &client_addr); // Records given client as logged in.
    bool                        ValidPassword(const AccountInfo &client, const char *password); // Verifies entered password matches stored password

    bool                        fill_account_info(AccountInfo &client);// Refresh client object from database
    int                         SaveAccount(const char *username, const char *password); // Save user account credentials to storage
    int                         RemoveAccount(AccountInfo &client); // Removes account from database via id #

    //bool AccountBlocked(const char *login) const; // Check if account is blocked.
    int                         AddIPBan(const ACE_INET_Addr &client_addr); // Add client's IP to the banlist.
    void                        InvalidGameServerConnection(const ACE_INET_Addr &);

    bool                        ReadConfig(const std::string &name); // later name will be used to read GameServer specific configuration
    bool                        Run(void);
    bool                        ShutDown(const std::string &reason="No particular reason");
    bool						Online(void);
    // Internal World-cluster interface
    ServerHandle<IGameServer>   RegisterMapServer(const ServerHandle<IMapServer> &map_h );
    int                         GetAccessKeyForServer(const ServerHandle<IMapServer> &h_server );
    // Internal Admin server interface
    CharacterDatabase *         character_db(){return m_char_db;}

protected:
    bool                        m_running;
    std::list<ACE_INET_Addr>    m_ban_list;
    AdminDatabase *             m_db;
    CharacterDatabase *         m_char_db;
};
typedef ACE_Singleton<_AdminServer,ACE_Thread_Mutex> AdminServer; // AdminServer Interface

#endif // ADMINSERVER_H
