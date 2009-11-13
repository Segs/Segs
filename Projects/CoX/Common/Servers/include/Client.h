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
// skeleton class used during authentication
class GameServerInterface;
class IClient
{
public:
	typedef enum
	{
		CLIENT_DISCONNECTED=0,
		CLIENT_EXPECTED,
		CLIENT_CONNECTED,
	} eClientState;
	IClient() :m_id(0),m_access_level(0),m_login(""),m_state(CLIENT_DISCONNECTED){}
	u64				getId()						{return m_id;}; // returns database id for this account
	void			setId(u64 nid)				{m_id=nid;}
	u16				getAccessLevel() const		{return m_access_level;}; // returns database id for this account
	void			setAccessLevel(u16 level)	{m_access_level=level;}
	string			getLogin()	const	{return m_login;}
	void			setLogin(const string &lgn)	{m_login=lgn;}
	void			setState(eClientState s) {m_state=s;}
	eClientState	getState() {return m_state;}
protected:
	u64 m_id;
	u16 m_access_level;
	std::string m_login;
	eClientState m_state;
}; 
class Entity;
class Client : public IClient
{
	ACE_INET_Addr m_peer_addr;
public:
	Client(){}
	u32				hash_id() {return m_peer_addr.get_ip_address()^m_peer_addr.get_port_number();}
	void			setPeer(const ACE_INET_Addr &peer){m_peer_addr=peer;}
	ACE_INET_Addr &	getPeer() {return m_peer_addr;}

// Fix? --malign
//        void                    setCharEntity(Entity *ent)=0;
//        Entity*                 getCharEntity()=0;
//          void                    setCharEntity(Entity *ent);
//          Entity*                 getCharEntity();
// End compile fix

	typedef enum
	{
		LOGGED_IN = 0,
		NOT_LOGGEDIN,
	} eClientState; // this is a public type 
};
class CharacterData
{

};

#endif // CLIENT_H
