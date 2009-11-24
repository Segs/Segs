/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.h 319 2007-01-26 17:03:18Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <ace/OS_NS_time.h>
#include "ServerManager.h"
#include "CharacterDatabase.h"
class Entity;
class Client // this should really be a simple 1-1 join between account info and connection, where depending on server only the link changes
{
public:
    typedef enum
    {
        CLIENT_DISCONNECTED=0,
        CLIENT_EXPECTED,
        NOT_LOGGED_IN,
        LOGGED_IN,
        CLIENT_CONNECTED,
    } eClientState;
    Client(){}
	u32				hash_id() {return m_peer_addr.get_ip_address()^m_peer_addr.get_port_number();}
	void			setPeer(const ACE_INET_Addr &peer){m_peer_addr=peer;}
	ACE_INET_Addr &	getPeer() {return m_peer_addr;}

    string			getLogin()	const	{return m_account_info.login();}
    void			setState(eClientState s) {m_state=s;}
    eClientState	getState() {return m_state;}
    bool            account_blocked() {return m_account_info.access_level()==0;}
    AccountInfo &   account_info() {return m_account_info;}
protected:
    ACE_INET_Addr m_peer_addr;
    eClientState m_state;
    AccountInfo m_account_info;
};
class CharacterData
{

};

#endif // CLIENT_H
