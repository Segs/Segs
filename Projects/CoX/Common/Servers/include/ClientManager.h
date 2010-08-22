/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include "Client.h"
#include "hashmap_selector.h"

template <class CLIENT_CLASS>
class ClientStore
{
	//	boost::object_pool<CLIENT_CLASS> m_pool;
	hash_map<u32,CLIENT_CLASS *> m_expected_clients;
	hash_map<u64,CLIENT_CLASS *> m_clients; // this maps client's id to it's object
	hash_map<u32,CLIENT_CLASS *> m_connected_clients_cookie; // this maps client's id to it's object
	hash_map<u64,u32> m_id_to_cookie; // client cookie is only useful in this context
	u32 create_cookie(const ACE_INET_Addr &from,u64 id)
	{
		u64 res = ((from.hash()+id&0xFFFFFFFF)^(id>>32));
		ACE_DEBUG ((LM_WARNING,ACE_TEXT ("(%P|%t) create_cookie still needs a good algorithm.0x%08x\n"),res));
		return (u32)res;
	};

public:

	CLIENT_CLASS *getById(u64 id)
	{
		if(m_clients.find(id)==m_clients.end())
			return NULL;
		return m_clients[id];
	}

	CLIENT_CLASS *getByCookie(u32 cookie)
	{
		if(m_connected_clients_cookie.find(cookie)!=m_connected_clients_cookie.end())
		{
			return m_connected_clients_cookie[cookie];
		}
		return NULL;
	}

	CLIENT_CLASS *getExpectedByCookie(u32 cookie)
	{
		// we got cookie check if it's an expected client
		if(m_expected_clients.find(cookie)!=m_expected_clients.end())
		{
			return m_expected_clients[cookie];
		}
		return NULL;
	}

	u32 ExpectClient(const ACE_INET_Addr &from,u64 id,u8 access_level)
	{
		CLIENT_CLASS * exp;
		u32 cook = create_cookie(from,id);
		// if we already expect this client
		if(m_expected_clients.find(cook)!=m_expected_clients.end())
		{
			// return pregenerated cookie
			return cook;
		}
		if(getByCookie(cook)) // already connected ?!
		{
			//
			return ~0; // invalid cookie
		}
		exp = new CLIENT_CLASS;
		exp->account_info().access_level(access_level);
		exp->account_info().account_server_id(id);
		exp->link_state().setState(ClientLinkState::CLIENT_EXPECTED);
		m_expected_clients[cook] = exp;
		m_clients[id] = exp;
		m_id_to_cookie[id]=cook;
		return cook;
	}
	void removeById(u64 id)
	{
		ACE_ASSERT(getById(id)!=0);
		CLIENT_CLASS *cl=getById(id);
		u32 cookie = m_id_to_cookie[id];

		m_id_to_cookie.erase(id);
		m_expected_clients.erase(cookie);
		m_connected_clients_cookie.erase(cookie);
		m_clients.erase(id);

		delete cl;
	}
	void connectedClient(u32 cookie)
	{
		// client with cookie has just connected
		m_connected_clients_cookie[cookie]=getExpectedByCookie(cookie);
		// so he's no longer expected
		m_expected_clients.erase(cookie);
	}
};

#endif // CLIENTMANAGER_H
