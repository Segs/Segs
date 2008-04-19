/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GameServer.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "ConfigExtension.h"
#include "GameServer.h"
#include "GameServerEndpoint.h"
#include "GamePacket.h"
#include "CharacterClient.h"
#include "CharacterDatabase.h"
#include "AdminServerInterface.h"

GameServer::GameServer(void) : 
	m_id(0),
	m_current_players(0),
	m_max_players(0),
	m_unk1(0),
	m_unk2(0),
	m_serverName(""),
	m_endpoint(NULL),
	m_online(false)
{
	m_database = new CharacterDatabase;
}

GameServer::~GameServer(void)
{
	if(ACE_Reactor::instance())
	{
		ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK);
		delete m_endpoint;
	}
	
}
bool GameServer::Run()
{
	if(m_endpoint)
	{
		ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already running\n") ));
		return true;
	}
	if(0!=m_database->OpenConnection())
	{
		ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: database connection failure\n"),false);
	}
	m_endpoint = new GameServerEndpoint(m_listen_point,this);
	if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
		ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
	m_online = true;
	return true;
}
bool GameServer::ReadConfig(const std::string &inipath)
{
	if(m_endpoint)
	{
		ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Game server already initialized and running\n") ));
		return true;
	}
	StringsBasedCfg config;
	if (config.open () == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("config")),false);
	}
	ACE_Ini_ImpExp config_importer (config);
	ACE_Configuration_Section_Key root;
	if (config_importer.import_config (inipath.c_str()) == -1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) GameServer: Unable to open config file : %s\n"), inipath.c_str()),false);
	if(-1==config.open_section(config.root_section(),"GameServer",1,root))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p GameServer: Config file %s is missing [GameServer] section\n"), inipath.c_str()),false);

	config.get_addr(root,ACE_TEXT("listen_addr"),m_listen_point,ACE_INET_Addr(7002,"0.0.0.0"));
	config.get_addr(root,ACE_TEXT("location_addr"),m_location,ACE_INET_Addr(7002,"127.0.0.1"));
	config.get_string_value(root,ACE_TEXT("server_name"),m_serverName,"unnamed");
	config.get_integer_value(root,ACE_TEXT("max_players"),m_max_players,600);
	string gamedb_host,gamedb_name,gamedb_user,gamedb_pass;
	config.get_string_value(root,ACE_TEXT("db_host"),gamedb_host,"127.0.0.1");
	config.get_string_value(root,ACE_TEXT("db_name"),gamedb_name,"none");
	config.get_string_value(root,ACE_TEXT("db_user"),gamedb_user,"none");
	config.get_string_value(root,ACE_TEXT("db_pass"),gamedb_pass,"none");

	m_database->setConnectionConfiguration(gamedb_host.c_str(),gamedb_name.c_str(),gamedb_user.c_str(),gamedb_pass.c_str());

	m_current_players = 0;
	m_id = 1;
	m_unk1=m_unk2=0;
	m_online = false;
	//m_db = new GameServerDb("");
	return true;
}
bool GameServer::ShutDown(const std::string &reason)
{
	if(!m_endpoint)
	{
		ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Server not running yet\n") ));
		return true;
	}
	m_database->CloseConnection();
	m_online = false;
	ACE_DEBUG((LM_WARNING,ACE_TEXT ("(%P|%t) Shutting down game server because : %s\n"), reason.c_str()));
	if (ACE_Reactor::instance()->remove_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
	{
		delete m_endpoint;
		ACE_ERROR_RETURN ((LM_ERROR, "ACE_Reactor::remove_handler"),false);
	}
	delete m_endpoint;
	return true;
}
// This method is called by authentication service, to notify this GameServer that a client 
// with given source ip/port,id and access_level has just logged in.
// If given client is not already logged in
//		This method will create a new CharacterClient object, put it in m_expected_clients collection, and return a key (u32) 
//		that will be used by the client during connection
//		Also this will set m_expected_clients cleaning timer if it isn't set already
// If given client is logged in ( it can be found here, or any other GameServer )
//		
// In return caller gets an unique client identifier. which is used later on to retrieve appropriate 
// client object
u32 GameServer::ExpectClient(const ACE_INET_Addr &from,u64 id,u16 access_level)
{
	return m_clients.ExpectClient(from,id,access_level);
}
CharacterClient * GameServer::ClientExpected(ACE_INET_Addr &from,pktCS_ServerUpdate *pak)
{
	CharacterClient * res = m_clients.getExpectedByCookie(pak->authCookie);
	if(res->getState()==IClient::CLIENT_EXPECTED)
	{
		m_clients.connectedClient(pak->authCookie);
		res->setState(IClient::CLIENT_CONNECTED);
	}
	// if (res->getExpectedPeer()==from)
	return res;
}
/*
u32 GameServer::GetClientCookie(const ACE_INET_Addr &client_addr)
{
	static u32 cookie=0;
	if(!m_endpoint)
	{
		ACE_ASSERT(ACE_TEXT("(%P|%t) Server not running yet\n") );
		return true;
	}
//	my_endpoint->
	return client_addr.get_ip_address()+(cookie++);
}
*/
void GameServer::checkClientConnection(u64 id)
{
//	m_clients.getById(id)->;
// empty for now, later on it will use client store to get the client, and then check it's packet backlog
//
}
bool GameServer::isClientConnected(u64 id)
{
	return m_clients.getById(id)!=NULL;
}
int GameServer::GetAccessKeyForServer(const ServerHandle<IMapServer> &h_map)
{
	return 0;
}
bool GameServer::MapServerReady(const ServerHandle<IMapServer> &h_map)
{
	return false;
}

std::string GameServer::getName( void )
{
	return m_serverName;
}

u8 GameServer::getId( void )
{
	return m_id;
}

u16 GameServer::getCurrentPlayers( void )
{
	return m_current_players;
}

u16 GameServer::getMaxPlayers()
{
	return m_max_players;
}

u8 GameServer::getUnkn1( void )
{
	return m_unk1;
}

u8 GameServer::getUnkn2( void )
{
	return m_unk2;
}

CharacterDatabase * GameServer::getDb()
{
	return m_database;
}
//
void GameServer::disconnectClient( IClient *cl )
{
	m_clients.removeById(cl->getId());
}

int GameServer::CreateLinkedAccount(u64 auth_account_id,const std::string &username)
{
	return m_database->CreateLinkedAccount(auth_account_id,username);
}