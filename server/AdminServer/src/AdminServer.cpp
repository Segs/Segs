/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminServer.cpp 306 2007-01-23 12:16:25Z nemerle $
 */

// segs includes
#include "AdminServer.h"
#include "AdminDatabase.h"
#include "Client.h"
#include "ConfigExtension.h"
// AdminServer should pull client data from database or from it's local, in-memory cache
// currently there is no such thing, and 'client-cache' is just a hash-map

// Defined constructor
AdminServer::AdminServer(void) : m_running(false)
{
	m_db=new AdminDatabase;
}

// Defined destructor
AdminServer::~AdminServer(void)
{
	(void)ShutDown();
}
bool AdminServer::ReadConfig(const std::string &inipath)
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
	if(-1==config.open_section(config.root_section(),"AdminServer",1,root))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminServer: Config file %s is missing [AdminServer] section\n"), inipath.c_str()),false);
	string dbhost,dbname,dbuser,dbpass;
	config.get_string_value(root,ACE_TEXT("db_host"),dbhost,"127.0.0.1");
	config.get_string_value(root,ACE_TEXT("db_name"),dbname,"none");
	config.get_string_value(root,ACE_TEXT("db_user"),dbuser,"none");
	config.get_string_value(root,ACE_TEXT("db_pass"),dbpass,"none");

	m_db->setConnectionConfiguration(dbhost.c_str(),dbname.c_str(),dbuser.c_str(),dbpass.c_str());
	return true;
}
bool AdminServer::Run()
{
	if(m_db->OpenConnection())
		return false;
	m_running=true;
	return true;
}
bool AdminServer::ShutDown(const std::string &reason/* ="No particular reason" */)
{
	bool res;
	ACE_DEBUG ((LM_TRACE,ACE_TEXT("(%P|%t) Shutting down AdminServer %s\n"),reason.c_str()));
	m_running=false;
	res=m_db->CloseConnection()==0;
	delete m_db;
	return res;
}
void AdminServer::FillClientInfo(IClient *client)
{
	if((client->getLogin().size()>0) && (client->getId()==0)) // existing client
	{
		if(m_db->GetAccountByName(client,client->getLogin())==0)
			return;
	}
	if(client->getId()) // existing client
	{
		if(m_db->GetAccountById(client,client->getId())==0)
			return;
	}

}
bool AdminServer::Login(const IClient *client,const ACE_INET_Addr &client_addr)
{
	// Here we should log to the Db, a Login event for that client
	//client->setState(AuthClient::LOGGED_IN); modifying this should be done in AuthServer
	return true;
}

bool AdminServer::Logout(const IClient *client) const
{
	// Here we should log to the Db, a Logout event for that client
	//if(client)
	//	client->setState(AuthClient::NOT_LOGGEDIN);
	return true;
}

bool AdminServer::ValidPassword(const IClient *client ,const char *pass)
{
	return m_db->ValidPassword(client->getLogin().c_str(),pass);
}

int AdminServer::SaveAccount(const char *username, const char *password) 
{
	return m_db->AddAccount(username, password);  // Add the given account
}

/*
int AdminServer::RemoveAccount(IClient *client)
{
	return m_db->RemoveAccountByID(client->getId());
}
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

int AdminServer::AddIPBan(const ACE_INET_Addr &client_addr)
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
ServerHandle<GameServer> AdminServer::RegisterMapServer(const ServerHandle<MapServer> &map_h)
{
	// For each server, find number of free Map handlers, and player occupancy ( servers with low number of Maps, and low occupancy should be prioritised)
	return ServerHandle<GameServer>(0);
}


/**
 * Idea: All GetAccessKeyForServer methods create a new 'allowed access' entry in their callers table.
 *		 So each call using Handle, is marked/encoded with it, and can be verified as valid.
 * @return int
 * @param  map_h
 */
int AdminServer::GetAccessKeyForServer(const ServerHandle<MapServer> &map_h) 
{
	return 0;
}
