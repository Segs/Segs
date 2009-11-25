/* 
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminDatabase.cpp 302 2006-12-27 16:35:17Z malign $
 */

// segs includes
#include "AdminServer.h"
#include "AdminDatabase.h"
#include <sstream>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>
#include "Client.h"

int AdminDatabase::GetAccounts(void) const
{
	return 0;
}

int AdminDatabase::AddAccount(const char *username, const char *password,u16 access_level) // Add account and password to the database server
{
	std::stringstream query;
	u8 res=0;
	size_t escaped_len;
	u8 *escaped_pass = PQescapeBytea((u8 *)password,14,&escaped_len);

	query<<"INSERT INTO accounts (username,passw,access_level) VALUES ('"<<username<<"','"<<escaped_pass<<"',"<<access_level<<")";
	PQfreemem(escaped_pass);
	PGresult *pResult = PQexec(pConnection,query.str().c_str());   // Send our query to the PostgreSQL db server to process
	if (PQresultErrorMessage(pResult)[0] != 0)
	{
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Result status: %s\n"),PQresultErrorMessage(pResult))); // Why the query failed
		res=1;
	}
	PQclear(pResult); // Clear result
	return res;
}

/*
int AdminDatabase::RemoveAccountByID(u64 id) // Remove account by it's id #
{
	std::stringstream query;
	query<<"DELETE FROM accounts WHERE id = "<<id;
	if(execQuery(query.str()))
		return 0;
	return 1;
}
*/

/*
int AdminDatabase::RemoveAccountByName(char *username) // Remove account by it's username
{
	std::stringstream query;
	query<<"DELETE FROM accounts WHERE username = '"<<username<<"'";
	if(execQuery(query.str()))
		return 0;
	return 1;
}
*/

/*
int AdminDatabase::GetBanFlag(const char *username)
{
	DbResults res;
	std::stringstream query;
	query<<"SELECT blocked FROM accounts WHERE username = '"<<username<<"'";
	if(execQuery(query.str(),res))
	{
		if(res.num_rows()==0)
			return 1;
		return res.getRow(0).getColInt16("blocked");
	}
	return 1;
}
*/

bool AdminDatabase::ValidPassword(const char *username, const char *password)
{
    std::stringstream query;
	bool res=false;
    u8 *binary_password;
    size_t unescaped_len;
	query<<"SELECT passw FROM accounts WHERE username = '"<<username<<"';";

	PGresult *pResult = PQexec(pConnection,query.str().c_str());   // Send our query to the PostgreSQL db server to process
	char *res_msg = PQresultErrorMessage(pResult);
	if(res_msg&&(res_msg[0]!=0))
	{
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Result status: %s\n"),PQresultErrorMessage(pResult))); // Why the query failed
		return res;
	}
    binary_password=PQunescapeBytea((u8 *)PQgetvalue(pResult,0,0),&unescaped_len);
	ACE_ASSERT(unescaped_len<=16);
    PQclear(pResult); // Clear result
	if (memcmp(binary_password,password,unescaped_len) == 0)
		res = true;
	PQfreemem(binary_password);
	return res;
	}
bool AdminDatabase::GetAccountByName( AccountInfo &to_fill,const std::string &login )
{
	stringstream query;
	query<<"SELECT * FROM accounts WHERE username='"<<login<<"';";
	return GetAccount(to_fill,query.str());
}
bool AdminDatabase::GetAccountById( AccountInfo &to_fill,u64 id )
{
	stringstream query;
	query<<"SELECT * FROM accounts WHERE id='"<<id<<"';";
	return GetAccount(to_fill,query.str());
}

bool AdminDatabase::GetAccount( AccountInfo & client,const std::string &query )
{
    DbResults results;
    if(!execQuery(query,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminDatabase::GetAccount query %s failed. %s.\n"), query.c_str(),results.m_msg),false);
    if(results.num_rows()!=1)
        return false;
    DbResultRow r=results.getRow(0);
	struct tm creation;
	client.m_acc_server_acc_id  = (u64)r.getColInt64("id"); 
	client.m_login              = r.getColString("username"); 
    client.m_access_level       = r.getColInt16("access_level"); 
    creation                    = r.getTimestamp("creation_date");
//	client->setCreationDate(creation); 
	return 0;

}

int AdminDatabase::RemoveAccountByID( u64 id )
{
	return 0;
}