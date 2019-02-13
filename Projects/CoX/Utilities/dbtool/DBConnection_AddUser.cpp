/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBConnection.h"

#include "PasswordHasher.h"
#include "Logging.h"
#include "Settings.h"

#include <QtCore/QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

dbToolResult DBConnection::addAccount(const QString &username, const QString &password, uint16_t access_level)
{
    qCDebug(logSettings) << "AddAccount to database at" << m_config.m_db_path;

    if(!m_query.prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << m_query.lastError();
        return dbToolResult::QUERY_PREP_FAILED;
    }

    PasswordHasher hasher;
    QByteArray salt = hasher.generateSalt();
    QByteArray password_array = hasher.hashPassword(password.toUtf8(), salt);
    m_query.bindValue(0, username);
    m_query.bindValue(1, password_array);
    m_query.bindValue(2, access_level);
    m_query.bindValue(3, salt);

    if(!m_query.exec())
    {
        int sqlErrorCode = m_query.lastError().nativeErrorCode().toInt();
        if(sqlErrorCode == 19 || sqlErrorCode == 1062 || sqlErrorCode == 23503)
        {
            // Unique constraint error.
            // SQLite:        19
            // MySQL:       1062
            // PostgreSQL: 23503
            qWarning() << "Error: Username already taken. Please try another name.";
            return dbToolResult::USERNAME_TAKEN;
        }
        qDebug() << "SQL_ERROR:" << m_query.lastError(); // Why the query failed
        return dbToolResult::QUERY_FAILED;
    }

    return dbToolResult::SUCCESS;
}
