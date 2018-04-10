/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
// segs includes
#include "Common/Servers/Server.h"
#include "Common/Servers/AdminServerInterface.h"

// ACE includes
#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

#include <list>
#include <string>


class AdminLink;
typedef ACE_Acceptor<AdminLink, ACE_SOCK_ACCEPTOR> AdminClientAcceptor;

class AdminDatabase;
class CharacterDatabase;
class IGameServer;
class IMapServer;
class AccountInfo;
//! The AdminServer class handles administrative functions such as account saving, account banning, etcetera.
class _AdminServer final : public IAdminServer
{
public:

    // Constructor/Destructor
                                _AdminServer(void);
                                ~_AdminServer(void) override;

    // Client handling related interface

    bool                        Login(const AccountInfo &client,const ACE_INET_Addr &client_addr) override;
    bool                        ValidPassword(const AccountInfo &client, const char *password) override;

    bool                        fill_account_info(AccountInfo &client) override;
    int                         SaveAccount(const char *username, const char *password) override;
    int                         RemoveAccount(AccountInfo &client) override; // Removes account from database via id #

    //bool AccountBlocked(const char *login) const; // Check if account is blocked.

    bool                        ReadConfig() override;
    bool                        Run(void) override;
    bool                        ShutDown(const QString &reason="No particular reason") override;
    // Internal Admin server interface
    CharacterDatabase *         character_db(){return m_char_db;}

protected:
    AdminClientAcceptor *       m_acceptor;     //!< ace acceptor wrapping AuthClientService
    ACE_INET_Addr               m_location;     //!< address this server will bind at.
    bool                        m_running;      //!< true if this server is running
    std::list<ACE_INET_Addr>    m_ban_list;     //!< list of banned IP addresses
    AdminDatabase *             m_db;           //!< account database access object
    CharacterDatabase *         m_char_db;      //!< character database access object
};
typedef ACE_Singleton<_AdminServer,ACE_Thread_Mutex> AdminServer; // AdminServer Interface
