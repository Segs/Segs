/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AuthServerInterface.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

#include "AuthServerInterface.h"
#include "AuthServer/include/AuthServer.h"
AuthServerInterface::AuthServerInterface(void)
{
	m_server = new AuthServer;
}

AuthServerInterface::~AuthServerInterface(void)
{
	delete m_server;
	m_server=NULL;
}
/*
void AuthServerInterface::SendError(int error) // called from auth server during user authentication, might be useful for automatical firewall rules update
{
	return m_server->SendError((AuthServer::eAuthError)error);
}
*/
AuthClient *AuthServerInterface::GetClientByLogin(const char *login)
{
	return m_server->GetClientByLogin(login);
}
bool AuthServerInterface::Run()
{
	return m_server->Run();
}
bool AuthServerInterface::ReadConfig(const std::string &name)
{
	return m_server->ReadConfig(name);
}
bool AuthServerInterface::ShutDown(const std::string &reason)
{
	return m_server->ShutDown(reason);
}
ServerHandle<AdminServer> AuthServerInterface::AuthenticateMapServer(const ServerHandle<MapServer> &map,int version,const string &passw)
{
    return m_server->AuthenticateMapServer(map,version,passw);
}
