/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
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
        ACE_ERROR((LM_ERROR,ACE_TEXT ("Result status: %s\n"),query_res.message())); // Why the query failed
        return false;
    }
    query_res.nextRow(); // sqlite3 needs this to call _step function
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
    DbResults qres;
    PreparedArgs user_args;
    bool res=false;

    user_args.add_param(username);

    if(!m_prepared_select_account_passw->execute(user_args,qres)) {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Result status: %s\n"),qres.message())); // Why the query failed
        return res;
    }
    IResultRow::vBinData passb = qres.nextRow().getColBinary("passw");
    assert(passb.size()<=16);
    if (memcmp(&passb[0],password,passb.size()) == 0)
        res = true;
    return res;
}
bool AdminDatabase::GetAccountByName( AccountInfo &to_fill,const std::string &login )
{
    DbResults results;
    PreparedArgs id_arg;

    id_arg.add_param(login);

    to_fill.m_acc_server_acc_id = 0;

    if(!m_prepared_select_account_by_username->execute(id_arg,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminDatabase::GetAccount query %s failed. %s.\n"),
                          m_prepared_select_account_by_username->prepared_sql().c_str(),results.message()),false);

    return GetAccount(to_fill,results);
}
bool AdminDatabase::GetAccountById( AccountInfo &to_fill,uint64_t id )
{
    DbResults results;
    PreparedArgs id_arg;

    id_arg.add_param(id);

    to_fill.m_acc_server_acc_id = 0;

    if(!m_prepared_select_account_by_id->execute(id_arg,results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) AdminDatabase::GetAccount query %s failed. %s.\n"),
                          m_prepared_select_account_by_id->prepared_sql().c_str(),results.message()),false);

    return GetAccount(to_fill,results);
}

bool AdminDatabase::GetAccount( AccountInfo & client,DbResults &results )
{
    DbResultRow r=results.nextRow();
    if(!r.valid()) // TODO: handle case of multiple accounts with same name ?
        return false;
    struct tm creation;
    client.m_acc_server_acc_id  = (uint64_t)r.getColInt64("id");
    client.m_login              = r.getColString("username");
    client.m_access_level       = uint8_t(r.getColInt16("access_level"));
    creation                    = r.getTimestamp("creation_date");
    //  client->setCreationDate(creation);
    return true;
}

AdminDatabase::~AdminDatabase()
{
    delete m_add_account_query;
    delete m_prepared_select_account_by_id;
    delete m_prepared_select_account_passw;
    delete m_prepared_select_account_by_username;

}

int AdminDatabase::RemoveAccountByID( uint64_t )
{
    return 0;
}

void AdminDatabase::on_connected(Database *db)
{
    if(!m_db)
        m_db=db;
    m_add_account_query = db->prepare("INSERT INTO accounts (username,passw,access_level) VALUES ($1,$2,$3);",3);
    if(!m_add_account_query)
        abort();
    m_prepared_select_account_by_username = db->prepare("SELECT * FROM accounts WHERE username=$1",1);
    m_prepared_select_account_by_id = db->prepare("SELECT * FROM accounts WHERE id=$1",1);
    m_prepared_select_account_passw = db->prepare("SELECT passw FROM accounts WHERE username = $1",1);
}
