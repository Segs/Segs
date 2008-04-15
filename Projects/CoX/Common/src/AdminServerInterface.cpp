/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminServerInterface.cpp 301 2006-12-26 15:50:44Z nemerle $
 */

#include "AdminServerInterface.h"
#include "AdminServer/include/AdminServer.h"
AdminServerInterface::AdminServerInterface(void)
{
}

AdminServerInterface::~AdminServerInterface(void)
{
}
int AdminServerInterface::GetBlockedIpList(std::list<int> &addreses) // called from auth server during user authentication, might be useful for automatical firewall rules update
{
	return IAdminServer::instance()->GetBlockedIpList(addreses);
}
bool AdminServerInterface::Login(IClient  *client,const ACE_INET_Addr &client_addr) // Records given 'client' as logged in from 'addr'.
{
	return IAdminServer::instance()->Login(client,client_addr);
}
bool AdminServerInterface::Logout(IClient  *client)
{
	return IAdminServer::instance()->Logout(client);
}
int AdminServerInterface::SaveAccount(const char *username, const char *password)
{
        return IAdminServer::instance()->SaveAccount(username, password);
}
bool AdminServerInterface::ValidPassword(const IClient *client, const char *password)
{
	return IAdminServer::instance()->ValidPassword(client, password);
}
void AdminServerInterface::InvalidGameServerConnection(const ACE_INET_Addr &from)
{
	return IAdminServer::instance()->InvalidGameServerConnection(from);
}
void AdminServerInterface::FillClientInfo(IClient *client)
{
	return IAdminServer::instance()->FillClientInfo(client);
}
bool AdminServerInterface::Run()
{
	return IAdminServer::instance()->Run();
}
bool AdminServerInterface::ReadConfig(const std::string &name)
{
	return IAdminServer::instance()->ReadConfig(name);
}
bool AdminServerInterface::ShutDown(const std::string &reason)
{
	return IAdminServer::instance()->ShutDown(reason);
}
ServerHandle<GameServer> RegisterMapServer(const ServerHandle<MapServer> &map_h)
{
	return IAdminServer::instance()->RegisterMapServer(map_h);
}
int GetAccessKeyForServer(const ServerHandle<MapServer> &h_server)
{
	return IAdminServer::instance()->GetAccessKeyForServer(h_server);
}

/*
void hashtest()
{
	u8 hash[16] = {0};
	//HashPassword(password, hash, sizeof(hash)); for now just set some arbitrary values in 'hash'
	char szSeed[(sizeof(u32) * 3) + 1] = {0};
	itoa(AuthNet::GetSeed(), szSeed, 10); //GetSeed() is just a func that returns int, set it to whatever you like
	MD5 md5;
	u8 digest[MD5::DIGESTSIZE];
	md5.Update(hash, sizeof(hash)); 
	md5.Update((u8 *)szSeed, strlen(szSeed)); 
	md5.Final(digest);// this is garble-2
	/ *
		garble1 ->  md5.Update(hash, sizeof(hash)); md5.Final(garble1);
		garble2 ->  md5.Update(hash, sizeof(hash)); md5.Update((u8 *)szSeed, strlen(szSeed)); md5.Final(garble2);
		garble3 ->  md5.Update(garble1, sizeof(garble1)); md5.Final(garble3);
		Important thing, check if garble3==garble2
	* /
}
*/
