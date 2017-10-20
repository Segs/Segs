/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "AuthServerInterface.h"
AuthServerInterface::AuthServerInterface(IAuthServer *server) : m_server(server)
{
}

AuthServerInterface::~AuthServerInterface()
{
    delete m_server;
    m_server=nullptr;
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
ServerHandle<IAdminServer> AuthServerInterface::AuthenticateMapServer(const tMapHandle &map, int version, const std::string &passw)
{
    return m_server->AuthenticateMapServer(map,version,passw);
}
