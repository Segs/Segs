/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "UpgradeDB.h"
#include "UpgradeHooks.h"
#include "CreateDB.h"
#include "DatabaseConfig.h"

#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

// doUpgrade
void doUpgrade(const std::vector<DatabaseConfig> &configs)
{
    qInfo() << "Upgrading database files...";

    // load newest database from templates to compare against
    std::vector<DatabaseConfig> tpls = configs;
    for(auto &tpl : tpls)
        tpl.m_dbname.append("_temp");

    // create temp databases
    dbToolResult tpl_result = createDatabases(tpls);
    if(tpl_result != dbToolResult::SUCCESS)
    {
        qWarning() << int(tpl_result)
                   << "Failed to read new database templates."
                   << "Please ensure that the template directory is properly configured.";
        return;
    }

    // Iterate over each database and compare to templates
    int i = 0;
    for(const DatabaseConfig &cfg : configs)
    {
        if(!dbExists(cfg))
        {
            qWarning() << "Database" << cfg.m_dbname
            << "does not exist! Please create using `create` command.";
            continue;
        }

        // check database version against schema in default folder
        VersionSchema current_db = getDBTableVersions(cfg);
        VersionSchema tpl_db     = getDBTableVersions(tpls.at(i));
        QStringList tables_to_update = getTablesToUpdate(current_db, tpl_db);

        if(current_db == tpl_db || tables_to_update.isEmpty())
        {
            qInfo() << "Database" << cfg.m_dbname << "is the latest version! No upgrade necessary.";
            continue;
        }

        // Otherwise we update database tables
        qInfo() << "Database is out of date! Attempting to upgrade now.";
        for(const QString &table : tables_to_update)
        {
            qInfo() << "Updating" << table;

        }

        /* PSUEDOCODE:
            // check database versions against template files
            auto current_db = getDatabaseVersion();
            auto tpl_db = checkTemplateVersion();
            getTablesToUpdate(current_db, tpl_db);

            // update database schema
            for(auto table : tables_to_update)
                update(table);

            // check columns for serialization versions
            checkColumnVersions();
            getColumnsToUpdate(current_db, tpl);

            // update those column schemas
            for(auto column : columns_to_update)
                update(column);

            // make sure we didn't mess up
            if(verifySchemas())
                return dbToolResult::SUCCESS;
        */
        ++i; // increment iterator for use in tpl vector
    }
}

QStringList getTablesToUpdate(const VersionSchema &cfg, const VersionSchema &tpl)
{
    QStringList tables_to_update;
    for(const QString &k : cfg.keys())
    {
        if(!tpl.contains(k))
        {
            qWarning() << "Template database does not contain key" << k;
            continue;
        }

        if(cfg.value(k) != tpl.value(k))
            tables_to_update.push_back(k);
    }
    return tables_to_update;
}

VersionSchema getDBTableVersions(const DatabaseConfig &cfg)
{
    VersionSchema result;
    // segs_db scope: wrap this section in brackets to prevent unwanted
    // removeDatabase() errors
    {
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(cfg.m_driver, cfg.m_dbname));

        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(cfg.m_dbname);
        segs_db.setHostName(cfg.m_host);
        segs_db.setPort(cfg.m_port.toInt());
        segs_db.setUserName(cfg.m_user);
        segs_db.setPassword(cfg.m_pass);

        if(!segs_db.open())
            qWarning() << "Failed to open database" << cfg.m_dbname;

        if(!segs_db.driver()->hasFeature(QSqlDriver::Transactions))
            qWarning() << segs_db.driverName() << "does not support Transactions";

        qCDebug(logDB) << "Table Versions for" << cfg.m_dbname.split(QDir::separator()).back();
        QString querytext = "SELECT table_name, version FROM table_versions";
        if(!query.exec(querytext))
        {
            qWarning() << "SQL_ERROR:" << query.lastError();
            qWarning() << "QUERY:" << query.executedQuery();
            return result;
        }

        while (query.next())
        {
            QString name = query.value(0).toString();
            int version = query.value(1).toInt();
            result.insert(name, version);
            qCDebug(logDB) << name << version;
        }
        query.finish();
        segs_db.close();

    } // end segs_db scope to prevent removeDatabase errors

    QSqlDatabase::removeDatabase(cfg.m_dbname);
    return result;
}
