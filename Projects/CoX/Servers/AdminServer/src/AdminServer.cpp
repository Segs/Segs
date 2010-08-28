/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// segs includes
#include "AdminServer.h"
#include "AdminDatabase.h"
#include "Client.h"
#include "ConfigExtension.h"
#include "CharacterDatabase.h"
// AdminServer should pull client data from database or from it's local, in-memory cache
// currently there is no such thing, and 'client-cache' is just a hash-map

// Defined constructor
_AdminServer::_AdminServer(void) : m_running(false)
{
    m_db        = new AdminDatabase;
    m_char_db   = new CharacterDatabase;
}

// Defined destructor
_AdminServer::~_AdminServer(void)
{
	(void)ShutDown();
}
bool _AdminServer::ReadConfig(const std::string &inipath)
{
	if(m_running)
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%P|%t) AdminServer: Already initialized and running\n") ),false);

	StringsBasedCfg config;
	if (config.open () == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("config")),false);
	}
	ACE_Ini_ImpExp config_importer (config);
	if (config_importer.import_config (inipath.c_str()) == -1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminServer: Unable to open config file : %s\n"), inipath.c_str()),false);
	ACE_Configuration_Section_Key root;
	ACE_Configuration_Section_Key account_db_config;
	ACE_Configuration_Section_Key character_db_config;
	if(-1==config.open_section(config.root_section(),"AdminServer",1,root))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminServer: Config file %s is missing [AdminServer] section\n"), inipath.c_str()),false);
	if(-1==config.open_section(root,"AccountDatabase",1,account_db_config))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminServer: Config file %s is missing [AccountDatabase] section\n"), inipath.c_str()),false);
	string dbhost,dbname,dbuser,dbpass,dbport;
	config.get_string_value(account_db_config,ACE_TEXT("db_host"),dbhost,"127.0.0.1");
	config.get_string_value(account_db_config,ACE_TEXT("db_port"),dbport,"5432");
	config.get_string_value(account_db_config,ACE_TEXT("db_name"),dbname,"none");
	config.get_string_value(account_db_config,ACE_TEXT("db_user"),dbuser,"none");
	config.get_string_value(account_db_config,ACE_TEXT("db_pass"),dbpass,"none");

	m_db->setConnectionConfiguration(dbhost.c_str(),dbport.c_str(),dbname.c_str(),dbuser.c_str(),dbpass.c_str());

    if(-1==config.open_section(root,"CharacterDatabase",1,character_db_config))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminServer: Config file %s is missing [CharacterDatabase] section\n"), inipath.c_str()),false);

	config.get_string_value(character_db_config,ACE_TEXT("db_host"),dbhost,"127.0.0.1");
	config.get_string_value(character_db_config,ACE_TEXT("db_port"),dbport,"5432");
	config.get_string_value(character_db_config,ACE_TEXT("db_name"),dbname,"none");
	config.get_string_value(character_db_config,ACE_TEXT("db_user"),dbuser,"none");
	config.get_string_value(character_db_config,ACE_TEXT("db_pass"),dbpass,"none");
	m_char_db->setConnectionConfiguration(dbhost.c_str(),dbport.c_str(),dbname.c_str(),dbuser.c_str(),dbpass.c_str());
	return true;
}
bool _AdminServer::Run()
{
	if(m_db->OpenConnection())
		return false;
	if(m_char_db->OpenConnection())
		return false;
	m_running=true;
	return true;
}
bool _AdminServer::ShutDown(const std::string &reason/* ="No particular reason" */)
{
	bool res;
	ACE_DEBUG ((LM_TRACE,ACE_TEXT("(%P|%t) Shutting down AdminServer %s\n"),reason.c_str()));
	m_running=false;
	res  =  m_db->CloseConnection()==0;
	res &=	m_char_db->CloseConnection()==0;

    delete m_db;
    delete m_char_db;
    return res;
}
bool _AdminServer::fill_account_info( AccountInfo &client )
{
	if((client.login().size()>0) && (client.account_server_id()==0)) // existing client
	{
		return m_db->GetAccountByName(client,client.login());
	}
	if(client.account_server_id()) // existing client
	{
		return m_db->GetAccountById(client,client.account_server_id());
	}
	return false;

}
bool _AdminServer::Login(const AccountInfo &,const ACE_INET_Addr &)
{
	// Here we should log to the Db, a Login event for that client
	//client->setState(AuthClient::LOGGED_IN); modifying this should be done in AuthServer
	return true;
}

bool _AdminServer::Logout(const AccountInfo &) const
{
	// Here we should log to the Db, a Logout event for that client
	//if(client)
	//	client->setState(AuthClient::NOT_LOGGEDIN);
	return true;
}

bool _AdminServer::ValidPassword( const AccountInfo &client, const char *password )
{
	return m_db->ValidPassword(client.login().c_str(),password);
}

int _AdminServer::SaveAccount(const char *username, const char *password)
{
	int res=0;
	if(false==m_db->AddAccount(username, password))
		return 1;

	AccountInfo tmp;
	tmp.login(username); // Fix if username is preprocessed before db entry in AddAccount
	fill_account_info(tmp);
	// also register this account on all currently available gameservers
	// they really should check for sync with AdminDb on startup
	//for(size_t idx=0; idx<ServerManager::instance()->GameServerCount(); idx++)
	//{
	// TODO: support multiple game servers.
		if(!m_char_db->CreateLinkedAccount(tmp.account_server_id(),username))
			res=2;
	//}
	return res;   // Add the given account
}

int _AdminServer::RemoveAccount(AccountInfo &client)
{
    int res = m_db->RemoveAccountByID(client.account_server_id());
    if(0==res)
        res = m_char_db->remove_account(client.game_server_id());
    return res;
}

/*
*/

/*
bool AdminServer::AccountBlocked(const char *login) const
{
	IClient *cl = GetClientByLogin(login);
	if(cl)
	{
		if(cl->getState()!=IClient::ACCOUNT_BLOCKED)
			return false; // Account is not blocked. Allow account to be used.
	}
	return true; // Account is blocked, or there is no such account. Deny them access.
}
*/

int _AdminServer::AddIPBan(const ACE_INET_Addr &client_addr)
{
	m_ban_list.push_back(client_addr);
	return 0;
}

/**
 * This method will find the most occupied GameServer, and attach MapServer bundle
 * (map_h) to it.
 * @return ServerHandle
 * @param  map_h
 */
ServerHandle<IGameServer> _AdminServer::RegisterMapServer(const ServerHandle<IMapServer> &map_h)
{
	// For each server, find number of free Map handlers, and player occupancy ( servers with low number of Maps, and low occupancy should be prioritized)
	return ServerHandle<IGameServer>(0);
}


/**
 * Idea: All GetAccessKeyForServer methods create a new 'allowed access' entry in their callers table.
 *		 So each call using Handle, is marked/encoded with it, and can be verified as valid.
 * @return int
 * @param  map_h
 */
int _AdminServer::GetAccessKeyForServer(const ServerHandle<IMapServer> &map_h)
{
    return 0;
}

bool _AdminServer::Online( void )
{
    return m_running;
}

void _AdminServer::InvalidGameServerConnection( const ACE_INET_Addr & )
{

}

int _AdminServer::GetBlockedIpList( std::list<int> & ) /* Called from auth server during user authentication, might be useful for automatical firewall rules update */
{
    return 0;
}
