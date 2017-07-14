/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

// segs includes
#include "AuthServer.h"

#include "ConfigExtension.h"
#include "AdminServer/AccountInfo.h"
#include "Common/Servers/AdminServerInterface.h"
#include "Common/Servers/ServerManager.h"
#include "AuthLink.h"
#include "AuthHandler.h"
#include "AuthClient.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

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
    if (!QFile::exists(inipath.c_str()))
    {
        qCritical() << "Config file" << inipath.c_str() <<"does not exist.";
        return false;
    }
    QSettings config(inipath.c_str(),QSettings::IniFormat);

    config.beginGroup("AuthServer");
    QString location_addr = config.value("listen_addr","127.0.0.1:2106").toString();
    if(!parseAddress(location_addr,m_location))
    {
        qCritical() << "Badly formed IP address" << location_addr;
        return false;
    }
    config.endGroup();
    return true;
}
/*!
 * @brief Starts this server up, by opening the connection acceptor on given location.
 * @return bool, if it's false, we somehow failed to start. Error report sis logged by ACE_ERROR_RETURN
 */
bool AuthServer::Run()
{
    AuthLink::g_target = new AuthHandler;
    AuthLink::g_target->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,2);
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
bool AuthServer::ShutDown(const std::string &/* ="No particular reason" */)
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
    AdminServerInterface *adminserv;                            // this will be used in case when we don't have this client in the cache
    hmClients::const_iterator iter = m_clients.find(login); // searching for the client in cache
    if(iter!=m_clients.end())               // if found
        return (*iter);                                //  return cached object
    adminserv = ServerManager::instance()->GetAdminServer();
    assert(adminserv);
    res= new AuthClient; //res= m_client_pool.construct();      // construct a new instance
    res->account_info().login(login);                           // set login and ask AdminServer to fill in the rest
    if( !adminserv->FillClientInfo(res->account_info()) )       // Can we fill the client account info from db ?
    {
        delete res;
        //m_client_pool.free(res);                              // nope ? Free object and return NULL.
        return NULL;
    }
    if(res->account_info().account_server_id()==0)              // check if object is filled in correctly, by validating it's db id.
    {
        delete res;
        //m_client_pool.free(res);                                // if it is not. Free object and return NULL.
        return NULL;
    }
    m_clients[res->account_info().login()]=res;                 // store valid object in the cache
    return res;
}
/**
 * @brief Performs whole authentication procedure.
 * @return ServerHandle
 * @param  map Initialized MapServerInterface
 * @param  version MapServer version
 * @param  passw server password
 */
ServerHandle<IAdminServer> AuthServer::AuthenticateMapServer(const ServerHandle<IMapServer> &/*map_h*/,int /*version*/,const std::string &/*passw*/)
{
    return ServerHandle<IAdminServer>(0);
}
