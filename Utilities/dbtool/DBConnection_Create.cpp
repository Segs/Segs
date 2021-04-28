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

#include "DatabaseConfig.h"
#include "DBConnection.h"

#include "Components/Logging.h"
#include "Components/Settings.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


/*!
 * @brief Creates a new database
 * @param[in] vector of database configuration settings
 * @returns enum class of return values
 */
DBToolResult DBConnection::createDB()
{
    qInfo() << "Creating database" << getName();

    if(!QFileInfo(m_config.m_template_path).isReadable())
    {
        qCritical() << m_config.m_template_path << "is not readable!"
                    << "Please check that the file is present and not corrupted.";
        return DBToolResult::SETTINGS_MISSING;
    }

    if(!deleteDB()) // Delete the existing database
    {
        qWarning(qPrintable(QString("FAILED to remove existing file: %1").arg(m_config.m_db_name)));
        qCritical("Ensure no processes are using it and you have permission to modify it.");
        return DBToolResult::DB_RM_FAILED;
    }

    QFile source_file(m_config.m_template_path);
    if(!runQueryFromFile(source_file))
        return DBToolResult::QUERY_FAILED;

    source_file.close();
    qInfo() << "COMPLETED creating:" << m_config.m_db_name;
    return DBToolResult::SUCCESS;
}

/*!
 * @brief deletes a database
 * @param[in] QString of database filename
 * @returns true if successful, false on failure
 */
bool DBConnection::deleteDB()
{
    if(m_config.isSqlite())
    {
        close(); // ensure SQLite dbase is truly closed or windows will be mad
        // SQLite databases are just a file. Remove it
        QFile target_file(m_config.m_db_name);
        if(target_file.exists() && !target_file.remove())
            return false;

        // For SQlite we must reopen database or it wont exist and
        // everything will fail without any real error messages
        close(); // yes, really
        
        // unload the database (this doesn't delete it)
        QSqlDatabase::removeDatabase(m_config.m_db_name);
        open(); // we just closed this, reopen it
    }
    else if(m_config.isMysql() || m_config.isPostgresql())
    {
        // Drop all tables
        QString query_text = QString("DROP TABLE [IF EXISTS] %1").arg(m_db->tables(QSql::AllTables).join(","));
        m_query->exec(query_text);
    }

    return true;
}

/*!
 * @brief Run queries from file on database
 * @param[in] Source file to read queries from
 * @param[in] Prepared query data to execute queries
 * @returns true if successful, false on failure
 */
bool DBConnection::runQueryFromFile(QFile &source_file)
{
    // Open file. If unsuccessful, return early.
    if(!source_file.open(QIODevice::ReadOnly))
    {
        qWarning().noquote() << "Query source file could not be opened.";
        return false;
    }

    // database scripts already have transactions, let's commit and close the open
    // transaction here before proceeding.
    if(!m_db->commit())
    {
        qWarning().noquote() << "Commit failed";
        return false;
    }

    // The SQLite driver executes only a single (the first) query in the QSqlQuery.
    // If the script contains more queries, it needs to be split.
    QStringList scriptQueries = QTextStream(&source_file).readAll().split(';');

    for(QString &q : scriptQueries) // Execute each command in the source file.
    {
        q = q.simplified(); // remove newlines and excess whitespace

        if(q.isEmpty())
            continue;

        if(!runQuery(q))
            return false;
    }

    return true;
}

//! @}
