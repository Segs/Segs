/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "AdminServerInterface.h"
AdminServerInterface::AdminServerInterface(IAdminServer *srv) : m_server(srv)
{
}

AdminServerInterface::~AdminServerInterface()
{
    m_server=nullptr;
}
//! Records given 'client' as logged in from 'addr'.
bool AdminServerInterface::Login(AuthAccountData &client, const ACE_INET_Addr &client_addr)
{
    return m_server->Login(client,client_addr);
}
//! Save account
int AdminServerInterface::SaveAccount(const char *username, const char *password)
{
        return m_server->SaveAccount(username, password);
}
//! If 'pass' is a valid password for client, return true
bool AdminServerInterface::ValidPassword(const AuthAccountData &client, const char *password)
{
    return m_server->ValidPassword(client, password);
}
bool AdminServerInterface::FillClientInfo(AuthAccountData &client)
{
    return m_server->fill_account_info(client);
}
bool AdminServerInterface::Run()
{
    return m_server->Run();
}
bool AdminServerInterface::ReadConfig()
{
    return m_server->ReadConfig();
}
bool AdminServerInterface::ShutDown(const QString &reason)
{
    return m_server->ShutDown(reason);
}
