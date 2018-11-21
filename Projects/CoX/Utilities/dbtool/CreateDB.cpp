/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "CreateDB.h"
#include "DatabaseConfig.h"

#include "Logging.h"
#include "Settings.h"

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
dbToolResult createDatabases(std::vector<DatabaseConfig> const& configs)
{
    qInfo() << "Creating databases...";
    dbToolResult ret = dbToolResult::SUCCESS;
    for(const DatabaseConfig &cfg : configs)
    {
        if(!QFileInfo(cfg.m_template_path).isReadable())
        {
            qCritical() << cfg.m_template_path << "is not readable!"
                        << "Please check that the file is present and not corrupted.";
            ret = dbToolResult::SETTINGS_MISSING;
            break;
        }
        QFile source_file(cfg.m_template_path);
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(cfg.m_driver, cfg.m_dbname));
        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(cfg.m_dbname);
        if(cfg.isSqlite())
        {
            // We have to remove the file if it already exists;
            // otherwise, many errors are thrown.
            if(!deleteDb(cfg.m_dbname))
            {
                qWarning(qPrintable(QString("FAILED to remove existing file: %1").arg(cfg.m_dbname)));
                qCritical("Ensure no processes are using it and you have permission to modify it.");
                ret = dbToolResult::DB_RM_FAILED;
                break;
            }
        }
        else if(cfg.isMysql() || cfg.isPostgresql())
        {
            segs_db.setHostName(cfg.m_host);
            segs_db.setPort(cfg.m_port.toInt());
            segs_db.setUserName(cfg.m_user);
            segs_db.setPassword(cfg.m_pass);
        }

        if(!segs_db.open())
        {
            qCritical("Could not open connection to database.\n Details:%s",
                      qPrintable(segs_db.lastError().text()));
            ret = dbToolResult::DB_CONN_FAILED;
            break;
        }

        if(!fileQueryDb(source_file, query))
        {
            // Roll back the database if something goes wrong,
            // so we're not left with useless poop.
            segs_db.rollback();
            qCritical("One of the queries failed to execute.\n Error detail: %s\n",
                      qPrintable(query.lastError().text()));
            ret = dbToolResult::QUERY_FAILED;
            break;
        }
        source_file.close();
        segs_db.close();
        qInfo() << "COMPLETED creating:" << cfg.m_dbname;
    }

    for(auto opened: configs)
        QSqlDatabase::removeDatabase(opened.m_dbname);

    return ret;
}

/*!
 * @brief deletes a database
 * @param[in] QString of database filename
 * @returns true if successful, false on failure
 */
bool deleteDb(QString const& db_file_name)
{
    QFile target_file(db_file_name);
    if(target_file.exists() && !target_file.remove())
        return false;

    return true;
}

/*!
 * @brief Run queries from file on database
 * @param[in] Source file to read queries from
 * @param[in] Prepared query data to execute queries
 * @returns true if successful, false on failure
 */
bool fileQueryDb(QFile &source_file, QSqlQuery &query)
{
    // Open file. If unsuccessful, return early.
    if(!source_file.open(QIODevice::ReadOnly))
    {
        qWarning().noquote() << "Query source file could not be opened.";
        return false;
    }
    // The SQLite driver executes only a single (the first) query in the QSqlQuery.
    // If the script contains more queries, it needs to be split.
    QStringList scriptQueries = QTextStream(&source_file).readAll().split(';');

    foreach(QString queryTxt, scriptQueries) // Execute each command in the source file.
    {
        if(queryTxt.trimmed().isEmpty())
            continue;

        if(!query.exec(queryTxt))
            return false;

        query.finish();
    }

    return true;
}

/*!
 * @brief Check database exists
 * @param[in] Config for database file
 * @returns true if successful, false on failure
 */
bool dbExists(const DatabaseConfig &dbcfg)
{
    bool ret = false;
    QString querytext;
    if(dbcfg.isSqlite())
    {
        // For SQLite we can just check that the file exists
        ret = fileExists(dbcfg.m_dbname);
    }
    else if(dbcfg.isMysql() || dbcfg.isPostgresql())
    {
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(dbcfg.m_driver,
                                                       dbcfg.m_dbname));
        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(dbcfg.m_dbname);
        segs_db.setHostName(dbcfg.m_host);
        segs_db.setPort(dbcfg.m_port.toInt());
        segs_db.setUserName(dbcfg.m_user);
        segs_db.setPassword(dbcfg.m_pass);
        segs_db.open();

        if(dbcfg.isMysql())
            querytext = "show tables";
        else
        {
            querytext = "SELECT table_schema || '.' || table_name FROM";
            querytext.append("information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema ='");
            querytext.append(dbcfg.m_dbname);
            querytext.append("';");
        }

        query.exec(querytext);
        if(query.size() >= 1)
            ret = true;

        segs_db.close();
    }

    QSqlDatabase::removeDatabase(dbcfg.m_dbname);
    return ret;
}
