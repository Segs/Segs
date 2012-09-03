/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <cassert>
#include <sstream>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>

// segs includes
#include "AdminServer.h"
#include "AdminDatabase.h"
#include "Client.h"
using namespace std;
AdminDatabase::AdminDatabase()
{

}
int AdminDatabase::GetAccounts(void) const
{
        return 0;
}

bool AdminDatabase::AddAccount(const char *username, const char *password,uint16_t access_level) // Add account and password to the database server
{
        PreparedArgs args;
        args.add_param(std::string(username));
        args.add_param((uint8_t*)password,14,true);
        args.add_param(access_level);
        DbResults query_res;
        if(false==m_add_account_query->execute(args,query_res)) // Send our query to the PostgreSQL db server to process
        {
                ACE_ERROR((LM_ERROR,ACE_TEXT ("Result status: %s\n"),query_res.m_msg)); // Why the query failed
                return false;
        }
        return true;
}

/*
int AdminDatabase::RemoveAccountByID(uint64_t id) // Remove account by it's id #
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
        uint8_t *binary_password;
        size_t unescaped_len;
        query<<"SELECT passw FROM accounts WHERE username = '"<<username<<"';";

        PGresult *pResult = PQexec(pConnection,query.str().c_str());   // Send our query to the PostgreSQL db server to process
        char *res_msg = PQresultErrorMessage(pResult);
        if(res_msg&&(res_msg[0]!=0))
        {
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Result status: %s\n"),PQresultErrorMessage(pResult))); // Why the query failed
                return res;
        }
        binary_password=PQunescapeBytea((uint8_t *)PQgetvalue(pResult,0,0),&unescaped_len);
        assert(unescaped_len<=16);
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
bool AdminDatabase::GetAccountById( AccountInfo &to_fill,uint64_t id )
{
        stringstream query;
        query<<"SELECT * FROM accounts WHERE id='"<<id<<"';";
        return GetAccount(to_fill,query.str());
}

bool AdminDatabase::GetAccount( AccountInfo & client,const std::string &query )
{
    DbResults results;
    client.m_acc_server_acc_id = 0;

    if(!execQuery(query,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminDatabase::GetAccount query %s failed. %s.\n"), query.c_str(),results.m_msg),false);
    if(results.num_rows()!=1)
        return false;

        DbResultRow r=results.getRow(0);
        struct tm creation;
        client.m_acc_server_acc_id  = (uint64_t)r.getColInt64("id");
        client.m_login              = r.getColString("username");
        client.m_access_level       = uint8_t(r.getColInt16("access_level"));
        creation                    = r.getTimestamp("creation_date");
//	client->setCreationDate(creation);
        return true;
}

int AdminDatabase::RemoveAccountByID( uint64_t )
{
        return 0;
}

void AdminDatabase::on_connected()
{
    m_add_account_query = new PreparedQuery(*this);
    m_add_account_query->prepare("INSERT INTO accounts (username,passw,access_level) VALUES ($1,$2,$3);",3);
}
