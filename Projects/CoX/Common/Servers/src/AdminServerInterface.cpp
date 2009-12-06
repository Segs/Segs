/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "AdminServerInterface.h"
AdminServerInterface::AdminServerInterface(IAdminServer *srv) : m_server(srv)
{
}

AdminServerInterface::~AdminServerInterface(void)
{
    m_server=0;
}
int AdminServerInterface::GetBlockedIpList(std::list<int> &addreses) // called from auth server during user authentication, might be useful for automatical firewall rules update
{
	return m_server->GetBlockedIpList(addreses);
}
bool AdminServerInterface::Login(AccountInfo &client,const ACE_INET_Addr &client_addr) // Records given 'client' as logged in from 'addr'.
{
	return m_server->Login(client,client_addr);
}
bool AdminServerInterface::Logout(AccountInfo &client)
{
	return m_server->Logout(client);
}
int AdminServerInterface::SaveAccount(const char *username, const char *password)
{
        return m_server->SaveAccount(username, password);
}
bool AdminServerInterface::ValidPassword(const AccountInfo &client, const char *password)
{
	return m_server->ValidPassword(client, password);
}
void AdminServerInterface::InvalidGameServerConnection(const ACE_INET_Addr &from)
{
	return m_server->InvalidGameServerConnection(from);
}
bool AdminServerInterface::FillClientInfo(AccountInfo &client)
{
	return m_server->fill_account_info(client);
}
bool AdminServerInterface::Run()
{
	return m_server->Run();
}
bool AdminServerInterface::ReadConfig(const std::string &name)
{
	return m_server->ReadConfig(name);
}
bool AdminServerInterface::ShutDown(const std::string &reason)
{
	return m_server->ShutDown(reason);
}
ServerHandle<IGameServer> AdminServerInterface::RegisterMapServer(const ServerHandle<IMapServer> &map_h)
{
	return m_server->RegisterMapServer(map_h);
}
int AdminServerInterface::GetAccessKeyForServer(const ServerHandle<IMapServer> &h_server)
{
	return m_server->GetAccessKeyForServer(h_server);
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
