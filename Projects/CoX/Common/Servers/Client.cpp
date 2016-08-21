/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "Client.h"

#include "Servers/AdminServer/AccountInfo.h"

ClientSession::ClientSession()
{
    m_account_info = new AccountInfo;
}

ClientSession::~ClientSession()
{
    delete m_account_info;
}

std::string ClientSession::getLogin() const   {
    return m_account_info->login();
}

bool ClientSession::account_blocked()   { return m_account_info->access_level()==0;}
