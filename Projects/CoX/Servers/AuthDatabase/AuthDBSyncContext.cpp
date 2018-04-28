/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup AuthDatabase Projects/CoX/Servers/AuthDatabase
 * @{
 */

#include "AuthDBSyncContext.h"

#include "AuthDBSyncEvents.h"
#include "PasswordHasher/PasswordHasher.h"
#include "Settings.h"

#include <ace/Thread.h>

#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <cassert>

namespace
{
    //! @image html dbschema/segs_dbschema.png
    bool GetAccount(RetrieveAccountResponseData &client, QSqlQuery &results)
    {
        if (!results.next()) // TODO: handle case of multiple accounts with same name ?
            return false;
        QDateTime creation;
        client.m_acc_server_acc_id = results.value("id").toULongLong();
        client.m_login             = results.value("username").toString();
        client.m_access_level      = results.value("access_level").toUInt();
        creation                   = results.value("creation_date").toDateTime();
        //  client->setCreationDate(creation);
        return true;
    }
} // namespace

AuthDbSyncContext::AuthDbSyncContext() {}

AuthDbSyncContext::~AuthDbSyncContext() {}

int64_t AuthDbSyncContext::getDbVersion(QSqlDatabase &db)
{
    QSqlQuery version_query(
        QStringLiteral("SELECT version FROM table_versions WHERE table_name='db_version' ORDER BY id DESC LIMIT 1"),
        db);
    if (!version_query.exec())
    {
        qDebug() << version_query.lastError();
        return -1;
    }
    if (!version_query.next())
        return -1;
    return version_query.value(0).toInt();
}

// Maybe one day we'll need to read different db configs per-thread, for now all just read the same file.
bool AuthDbSyncContext::loadAndConfigure()
{
    char          thread_name_buf[16];
    ACE_Thread_ID our_id;
    if (m_setup_complete)
    {
        qCritical() << "This AuthDbSyncContext has already been configured";
        return false;
    }

    qInfo() << "Loading AuthDbSync settings...";
    QSettings config(Settings::getSettingsPath(), QSettings::IniFormat, nullptr);

    config.beginGroup(QStringLiteral("AdminServer"));
    QStringList driver_list {"QSQLITE", "QPSQL", "QMYSQL"};
    our_id.to_string(thread_name_buf); // Ace is using template specialization to acquire the lenght of passed buffer

    config.beginGroup(QStringLiteral("AccountDatabase"));
    // this indent is here to mark the nesting of config block
    QString dbdriver = config.value(QStringLiteral("db_driver"), "QSQLITE").toString();
    QString dbhost   = config.value(QStringLiteral("db_host"), "127.0.0.1").toString();
    int     dbport   = config.value(QStringLiteral("db_port"), "5432").toInt();
    QString dbname   = config.value(QStringLiteral("db_name"), "segs").toString();
    QString dbuser   = config.value(QStringLiteral("db_user"), "segsadmin").toString();
    QString dbpass   = config.value(QStringLiteral("db_pass"), "segs123").toString();
    config.endGroup(); // CharacterDatabase
    config.endGroup(); // AdminServer

    if (!driver_list.contains(dbdriver.toUpper()))
    {
        qWarning() << "Database driver" << dbdriver << " not supported";
        return false;
    }
    QSqlDatabase *db2 =
        new QSqlDatabase(QSqlDatabase::addDatabase(dbdriver, QStringLiteral("AdminDatabase_") + thread_name_buf));
    db2->setHostName(dbhost);
    db2->setPort(dbport);
    db2->setDatabaseName(dbname);
    db2->setUserName(dbuser);
    db2->setPassword(dbpass);
    m_db.reset(db2); // at this point we become owner of the db
    if (!m_db->open())
    {
        qCritical().noquote() << "Failed to open database:" << dbname;
        return false;
    }
    int db_version = getDbVersion(*m_db);
    if (db_version != required_db_version)
    {
        qCritical() << "Wrong db version:" << db_version
                    << "this GameDatabase service requires:" << required_db_version;
        return false;
    }
    m_add_account_query.reset(new QSqlQuery(*m_db));
    m_prepared_select_account_by_username.reset(new QSqlQuery(*m_db));
    m_prepared_select_account_by_id.reset(new QSqlQuery(*m_db));
    m_prepared_select_account_passw.reset(new QSqlQuery(*m_db));

    if (!m_add_account_query->prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << m_add_account_query->lastError();
        return false;
    }
    if (!m_prepared_select_account_by_username->prepare("SELECT * FROM accounts WHERE username=?;"))
    {
        qDebug() << "SQL_ERROR:" << m_prepared_select_account_by_username->lastError();
        return false;
    }
    if (!m_prepared_select_account_by_id->prepare("SELECT * FROM accounts WHERE id=?;"))
    {
        qDebug() << "SQL_ERROR:" << m_prepared_select_account_by_id->lastError();
        return false;
    }
    if (!m_prepared_select_account_passw->prepare("SELECT passw,salt FROM accounts WHERE username = ?;"))
    {
        qDebug() << "SQL_ERROR:" << m_prepared_select_account_passw->lastError();
        return false;
    }

    return true;
}

bool AuthDbSyncContext::addAccount(const CreateAccountData &data)
{
    PasswordHasher hasher;
    QByteArray     salt            = hasher.generateSalt();
    QByteArray     hashed_password = hasher.hashPassword(data.password.toUtf8(), salt);
    m_add_account_query->bindValue(0, data.username);
    m_add_account_query->bindValue(1, hashed_password);
    m_add_account_query->bindValue(2, data.access_level);
    m_add_account_query->bindValue(3, salt);
    if (false == m_add_account_query->exec()) // Send our query to the PostgreSQL db server to process
    {
        last_error.reset(new QSqlError(m_add_account_query->lastError()));
        qDebug() << "SQL_ERROR:" << *last_error; // Why the query failed
        return false;
    }
    return true;
}

// bool AuthDbSyncContext::retrieveAccount(const RetrieveAccountRequestData &data,RetrieveAccountResponseData &result)
//{
//    result.m_acc_server_acc_id = 0;
//    if(data.m_id==0)
//    {
//        m_prepared_select_account_by_username->bindValue(0,data.m_login);
//        if(!m_prepared_select_account_by_username->exec()) {
//            last_error.reset(new QSqlError(m_prepared_select_account_by_username->lastError()));
//            qDebug() << "SQL_ERROR:"<< *last_error; // Why the query failed
//            return false;
//        }
//        return GetAccount(result,*m_prepared_select_account_by_username);
//    }
//    else
//    {
//        m_prepared_select_account_by_id->bindValue(0,data.m_id);
//        if(!m_prepared_select_account_by_id->exec()) {
//            last_error.reset(new QSqlError(m_prepared_select_account_by_id->lastError()));
//            qDebug() << "SQL_ERROR:"<< *last_error; // Why the query failed
//            return false;
//        }
//        return GetAccount(result,*m_prepared_select_account_by_username);
//    }
//}
bool AuthDbSyncContext::checkPassword(const QString &login, const QString &password)
{
    m_prepared_select_account_passw->bindValue(0, login);

    if (!m_prepared_select_account_passw->exec())
    {
        last_error.reset(new QSqlError(m_prepared_select_account_passw->lastError()));
        qDebug() << "SQL_ERROR:" << *last_error; // Why the query failed
        return false;
    }
    if (!m_prepared_select_account_passw->next())
    {
        return false;
    }
    PasswordHasher hasher;
    QByteArray     required_pass = m_prepared_select_account_passw->value("passw").toByteArray();
    QByteArray     salt          = m_prepared_select_account_passw->value("salt").toByteArray();
    // TODO: remove
    QByteArray hashed_password = hasher.hashPassword(password.toUtf8(), salt);
    return hashed_password == required_pass;
}
bool AuthDbSyncContext::retrieveAccountAndCheckPassword(const RetrieveAccountRequestData &data,
                                                        RetrieveAccountResponseData &     result)
{
    result.m_acc_server_acc_id = 0;
    assert(data.m_id == 0);
    if (!checkPassword(data.m_login, data.m_password))
    {
        if(!last_error)
        {
            result.m_acc_server_acc_id = 0;
            return true;
        }
        return !last_error->isValid();
    }
    m_prepared_select_account_by_username->bindValue(0, data.m_login);
    if (!m_prepared_select_account_by_username->exec())
    {
        last_error.reset(new QSqlError(m_prepared_select_account_by_username->lastError()));
        qDebug() << "SQL_ERROR:" << *last_error; // Why the query failed
        return false;
    }
    return GetAccount(result, *m_prepared_select_account_by_username);
}

bool AuthDbSyncContext::getPasswordValidity(const ValidatePasswordRequestData &data,
                                            ValidatePasswordResponseData &     result)
{
    result.m_valid_password = checkPassword(data.username, data.password);
    return !last_error->isValid();
}

//! @}
