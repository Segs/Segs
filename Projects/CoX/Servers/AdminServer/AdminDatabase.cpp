/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
// segs includes
#include "AdminServer.h"

#include "AccountInfo.h"
#include "AdminDatabase.h"
#include "Client.h"

#include <ace/Log_Msg.h>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <cassert>
#include <sstream>


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
    m_add_account_query.bindValue(0,username);
    m_add_account_query.bindValue(1,QByteArray::fromRawData(password,14));
    m_add_account_query.bindValue(2,access_level);
    if(false==m_add_account_query.exec()) // Send our query to the PostgreSQL db server to process
    {
        qDebug() << "SQL_ERROR:"<<m_add_account_query.lastError(); // Why the query failed
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
    bool res=false;
    m_prepared_select_account_passw.bindValue(0,username);

    if(!m_prepared_select_account_passw.exec()) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_passw.lastError(); // Why the query failed
        return false;
    }
    if(!m_prepared_select_account_passw.next())
        return false;
    QByteArray passb = m_prepared_select_account_passw.value("passw").toByteArray();
    assert(passb.size()<=16);
    if (memcmp(passb.data(),password,passb.size()) == 0)
        res = true;
    return res;
}
bool AdminDatabase::GetAccountByName( AccountInfo &to_fill,const QString &login )
{
    to_fill.m_acc_server_acc_id = 0;
    m_prepared_select_account_by_username.bindValue(0,login);
    if(!m_prepared_select_account_by_username.exec()) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_by_id.lastError();
        return false;
    }
    return GetAccount(to_fill,m_prepared_select_account_by_username);
}
bool AdminDatabase::GetAccountById( AccountInfo &to_fill,uint64_t id )
{
    to_fill.m_acc_server_acc_id = 0;
    m_prepared_select_account_by_id.bindValue(0,id);
    if(!m_prepared_select_account_by_id.exec()) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_by_id.lastError();
        return false;
    }

    return GetAccount(to_fill,m_prepared_select_account_by_id);
}

bool AdminDatabase::GetAccount( AccountInfo & client, QSqlQuery &results )
{
    if(!results.next()) // TODO: handle case of multiple accounts with same name ?
        return false;
    QDateTime creation;
    client.m_acc_server_acc_id  = results.value("id").toULongLong();
    client.m_login              = results.value("username").toString();
    client.m_access_level       = results.value("access_level").toUInt();
    creation                    = results.value("creation_date").toDateTime();
    //  client->setCreationDate(creation);
    return true;
}

AdminDatabase::~AdminDatabase()
{
}

int AdminDatabase::RemoveAccountByID( uint64_t )
{
    return 0;
}

void AdminDatabase::on_connected(QSqlDatabase *db)
{
    if(!m_db)
        m_db=db;
    m_add_account_query = QSqlQuery(*db);
    m_prepared_select_account_by_username = QSqlQuery(*db);
    m_prepared_select_account_by_id = QSqlQuery(*db);
    m_prepared_select_account_passw = QSqlQuery(*db);

    if(!m_add_account_query.prepare("INSERT INTO accounts (username,passw,access_level) VALUES (?,?,?);")) {
        qDebug() << "SQL_ERROR:"<<m_add_account_query.lastError();
        return;
    }
    if(!m_prepared_select_account_by_username.prepare("SELECT * FROM accounts WHERE username=?;")) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_by_username.lastError();
        return;
    }
    if(!m_prepared_select_account_by_id.prepare("SELECT * FROM accounts WHERE id=?;")) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_by_id.lastError();
        return;
    }
    if(!m_prepared_select_account_passw.prepare("SELECT passw FROM accounts WHERE username = ?;")) {
        qDebug() << "SQL_ERROR:"<<m_prepared_select_account_passw.lastError();
        return;
    }
}
