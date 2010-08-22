/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "ServerManager.h"
#include <ace/Message_Block.h>
#include "ConfigExtension.h"
#include "GameServer.h"
#include "CharacterDatabase.h"
#include "AdminServerInterface.h"
#include "GameHandler.h"
#include "Filesystem.h"

GameServer::GameServer(void) :
	m_online(false),
	m_id(0),
	m_current_players(0),
	m_max_players(0),
	m_unk1(0),
	m_unk2(0),
	m_serverName(""),
	m_endpoint(NULL)
{
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
	ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading game data\n") ));
	WorldData::instance()->read_costumes("./data/");
	WorldData::instance()->read_colors("./data/");
	ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) All game data read\n") ));
	ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Filling hashes .. ") ));
	WorldData::instance()->fill_hashes();
	ACE_DEBUG((LM_WARNING,ACE_TEXT("Hashes filled\n") ));

    m_handler = new GameHandler;
    m_handler->set_server(this);
    GameLink::g_target = m_handler;
    GameLink::g_target->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,2);

    m_endpoint = new ServerEndpoint<GameLink>(m_listen_point); //,this

    GameLink::g_link_target = m_endpoint;

	if (ACE_Reactor::instance()->register_handler(m_endpoint,ACE_Event_Handler::READ_MASK) == -1)
		ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ACE_Reactor::register_handle\n"),false);
	if (m_endpoint->open() == -1) // will register notifications with current reactor
		ACE_ERROR_RETURN ((LM_ERROR, "(%P|%t) GameServer: ServerEndpoint::open\n"),false);

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
int GameServer::getAccessKeyForServer(const ServerHandle<IMapServer> &h_map)
{
	return 0;
}
bool GameServer::isMapServerReady(const ServerHandle<IMapServer> &h_map)
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

//

