/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthServer.cpp 316 2007-01-25 15:17:16Z nemerle $
 */

// segs includes
#include "ConfigExtension.h"
#include "AuthServer.h"
#include "AuthClientServicer.h"
#include "AuthClient.h"
#include "InterfaceManager.h"


/*!
 * @class AuthServer
 * @brief main class of the authentication server.
 * 
 * @note AuthServer should pull client data from database or from it's local, 
 * in-memory cache currently there is no such thing, and 'client-cache' is just a hash-map
 */


AuthServer::AuthServer(void)
{
	m_acceptor = new ClientAcceptor;
	m_running=false;
}

AuthServer::~AuthServer(void)
{
	
	ShutDown();
	delete m_acceptor;
}
/*!
 * @brief Read server configuration
 * @param inipath is a path to our configuration file.
 * @return bool, if it's false, this function failed somehow.
 */
bool AuthServer::ReadConfig(const std::string &inipath)
{
	if(m_running)
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%P|%t) AuthServer: Already initialized and running\n") ),false);
	StringsBasedCfg config;
	if (config.open () == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("config")),false);
	}
	ACE_Ini_ImpExp config_importer (config);
	ACE_Configuration_Section_Key root;
	if (config_importer.import_config (inipath.c_str()) == -1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p AuthServer: Unable to open config file : %s\n"), inipath.c_str()),false);
	if(-1==config.open_section(config.root_section(),"AuthServer",1,root))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p AuthServer: Config file %s is missing [AuthServer] section\n"), inipath.c_str()),false);

	config.get_addr(root,ACE_TEXT ("listen_addr"),m_location,ACE_INET_Addr(2106,"127.0.0.1"));
	return true;
}
/*!
 * @brief Starts this server up, by opening the connection acceptor on given location.
 * @return bool, if it's false, we somehow failed to start. Error report sis logged by ACE_ERROR_RETURN
 */
bool AuthServer::Run()
{
	if (m_acceptor->open(m_location) == -1)
	{
		ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT ("%p\n"),ACE_TEXT ("Opening the Acceptor failed")), false);
	}
	m_running=true;
	return true;
}
/*!
 * @brief Shuts the server down
 * @param reason the value that should be stored the persistent log.
 * @return bool, if it's false, we failed to close down cleanly
 */
bool AuthServer::ShutDown(const std::string &reason/* ="No particular reason" */)
{
	m_acceptor->close();
	m_running=false;
	return true;
}

/*!
 * @brief Returns AuthClient corresponding to given login.
 * @param login - client's login.
 * @return if found, returns a valid AuthClient object, NULL otherwise.
 * If the client with given login exist in clients hash map return it, otherwise create this object and fill it from db.
 */
AuthClient *AuthServer::GetClientByLogin(const char *login)
{
	AuthClient *res=NULL;
	AdminServerInterface *adminserv;						// this will be used in case when we don't have this client in the cache
	hmClients::const_iterator iter = m_clients.find(login);	// searching for the client in cache
	if(iter!=m_clients.end())									// if found
		return ((*iter).second);							//	return cached object
	adminserv = ServerManager::instance()->GetAdminServer();
	ACE_ASSERT(adminserv);
	res= m_client_pool.construct();							// construct a new instance
	res->setLogin(login);									// set login
	adminserv->FillClientInfo(res);							// and ask AdminServer to fill in the rest
	if(res->getId()<=0)										// check if object is filled in correctly, by validating it's db id.
	{
		m_client_pool.free(res);							// if it is not. Free object and return NULL.
		return NULL;
	}
	m_clients[res->getLogin()]=res;							// store valid object in the cache
	return res;
}
/**
 * @brief Performs whole authentication procedure.
 * @return ServerHandle
 * @param  map Initialized MapServerInterface
 * @param  version MapServer version
 * @param  passw server password
 */
ServerHandle<AdminServer> AuthServer::AuthenticateMapServer(const ServerHandle<MapServer> &map,int version,const string &passw)
{
    // we need to ask our adminserver the key it's 
//    AdminServerInterface *i_admin = InterfaceManager::instance()->get(ServerHandle<AdminServer>(0)); // trying local only
    return ServerHandle<AdminServer>(0);
}
