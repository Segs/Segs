/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "AddUser.h"
#include "CreateDB.h"

#include "PasswordHasher.h"
#include "Logging.h"
#include "Settings.h"

#include <QtCore/QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

dbToolResult addAccount(const DatabaseConfig &char_database, const QString & username,
                const QString & password, uint16_t access_level)
{
    qCDebug(logSettings) << "AddAccount to database at" << char_database.m_db_path;
    QString target_file_string("");
    if(char_database.isSqlite())
    {
        target_file_string = char_database.m_db_path;
        QFile target_file(target_file_string);
        if(!target_file.exists())
        {
            qCritical("Target file could not be found. Verify its existence and try again.");
            return dbToolResult::SQLITE_DB_MISSING;
        }
    }
    else
        target_file_string = char_database.m_db_path;

    QSqlDatabase segs_db(QSqlDatabase::addDatabase(char_database.m_driver,
                                                   char_database.m_db_path));
    segs_db.setDatabaseName(target_file_string);
    if(char_database.isMysql() || char_database.isPostgresql())
    {
        segs_db.setHostName(char_database.m_host);
        segs_db.setPort(char_database.m_port.toInt());
        segs_db.setUserName(char_database.m_user);
        segs_db.setPassword(char_database.m_pass);
    }
    segs_db.open();
    QSqlQuery query(segs_db);
    if(!query.prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << query.lastError();
        return dbToolResult::QUERY_PREP_FAILED;
    }

    PasswordHasher hasher;
    QByteArray salt = hasher.generateSalt();
    QByteArray password_array = hasher.hashPassword(password.toUtf8(), salt);
    query.bindValue(0, username);
    query.bindValue(1, password_array);
    query.bindValue(2, access_level);
    query.bindValue(3, salt);

    if(!query.exec())
    {
        int sqlErrorCode = query.lastError().nativeErrorCode().toInt();
        if(sqlErrorCode == 19 || sqlErrorCode == 1062 || sqlErrorCode == 23503)
        {
            // Unique constraint error.
            // SQLite:        19
            // MySQL:       1062
            // PostgreSQL: 23503
            qDebug() << "Error: Username already taken. Please try another name.";
            return dbToolResult::USERNAME_TAKEN;
        }
        qDebug() << "SQL_ERROR:" << query.lastError(); // Why the query failed
        return dbToolResult::QUERY_FAILED;
    }
    query.finish();
    segs_db.close();
    return dbToolResult::SUCCESS;
}
