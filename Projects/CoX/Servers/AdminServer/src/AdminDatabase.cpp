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
	bool res=false;
	static char SQLStmt[256]; // Variable to hold our query
	sprintf(SQLStmt, "SELECT passw FROM accounts WHERE username = '%s'",username);
	u8 *binary_password;
	size_t unescaped_len;

	PGresult *pResult = PQexec(pConnection,SQLStmt);   // Send our query to the PostgreSQL db server to process
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
int AdminDatabase::GetAccountByName(IClient *client, const string &login)
{
	stringstream query;
	query<<"SELECT * FROM accounts WHERE username='"<<login<<"'"<<endl;
	return GetAccount(client,query.str());
}
int AdminDatabase::GetAccountById(IClient *client, u64 id)
{
	stringstream query;
	query<<"SELECT * FROM accounts WHERE id='"<<id<<"'";
	return GetAccount(client,query.str());
}

int AdminDatabase::GetAccount(IClient *client,const string &query)
{
	PGresult *pResult = PQexec(pConnection,query.c_str());   // Send our query to the PostgreSQL db server to process
	char *res_msg = PQresultErrorMessage(pResult);
	if(res_msg&&(res_msg[0]!=0))
	{
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Result status: %s\n"),res_msg)); // Why the query failed
		PQclear(pResult); // Clear result
		return 1;
	}
	if (PQntuples(pResult)!=1)
	{
		PQclear(pResult); // Clear result
		return 1; // too much, or too little rows
	}
	struct tm creation;
	// FIXME: This should be _atoi64 or atoll
	client->setId((unsigned int)atoi(PQgetvalue(pResult,0,0))); 
	client->setLogin(PQgetvalue(pResult,0,1)); 

	u8 *binary_password;
	size_t unescaped_len;
//	binary_password=PQunescapeBytea((u8 *)PQgetvalue(pResult,0,4),&unescaped_len);
//	client->setPassword(binary_password);
//	ACE_ASSERT(unescaped_len<=16);
//	PQfreemem(binary_password);

	client->setAccessLevel(atoi(PQgetvalue(pResult,0,2))); 
	(void)ACE_OS::strptime(PQgetvalue(pResult,0,3),"%Y-%m-%d %T",&creation);
//	client->setCreationDate(creation); 
	PQclear(pResult); // Clear result
	return 0;

}

int AdminDatabase::RemoveAccountByID( u64 id )
{
	return 0;
}