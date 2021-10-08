/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "DBConnection.h"

#include "Components/PasswordHasher.h"
#include "Components/Logging.h"

#include <QtCore/QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtCore/QDateTime>

/*!
 * @brief       Add an account to the database
 * @param[in]   Username as String
 * @param[in]   Plain text password to be hashed herein
 * @param[in]   Access Level ranging from 0-9
 * @returns     Returns an enum value based upon result, 0 == SUCCESS
 */
DBToolResult DBConnection::addAccount(const QString &username, const QString &password, uint16_t access_level)
{
    qInfo().noquote() << "Adding new account to" << getName() << "database";

    if(!m_query->prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << m_query->lastError();
        return DBToolResult::QUERY_PREP_FAILED;
    }

    PasswordHasher hasher;
    QByteArray salt = hasher.generateSalt();
    QByteArray password_array = hasher.hashPassword(password.toUtf8(), salt);
    m_query->bindValue(0, username);
    m_query->bindValue(1, password_array);
    m_query->bindValue(2, access_level);
    m_query->bindValue(3, salt);

    if(!m_query->exec())
    {
        int sqlErrorCode = m_query->lastError().nativeErrorCode().toInt();
        if(sqlErrorCode == 19 || sqlErrorCode == 1062 || sqlErrorCode == 23503)
        {
            // Unique constraint error.
            // SQLite:        19
            // MySQL:       1062
            // PostgreSQL: 23503
            qWarning() << "Error: Username already taken. Please try another name.";
            return DBToolResult::USERNAME_TAKEN;
        }
        qDebug() << "SQL_ERROR:" << m_query->lastError(); // Why the query failed
        return DBToolResult::QUERY_FAILED;
    }

    // attempt to commit transaction
    if(!m_db->commit())
    {
        qWarning() << "Commit unsuccessful, rolling back database.";
        m_db->rollback();
        return DBToolResult::QUERY_FAILED;
    }

    qInfo() << "Successfully added user" << username << "to database";
    return DBToolResult::SUCCESS;
}

//! @}
