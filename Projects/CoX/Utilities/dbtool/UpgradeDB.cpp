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
void runUpgrades(const std::vector<DatabaseConfig> &configs)
{
    qInfo() << "Upgrading database files...";

    // Iterate over each database and compare to upgradeHooks
    for(const DatabaseConfig &cfg : configs)
    {
        if(!dbExists(cfg))
        {
            // this should never happen, if it does we screwed up
            qWarning() << "Database" << cfg.m_db_path
            << "does not exist! Please create using `create` command.";
            return;
        }

        // check database version against schema in default folder
        TableSchema current_version = getDBVersion(cfg);
        if(!checkVersionAndUpgrade(cfg, current_version))
        {
            qInfo() << "Database" << cfg.m_name << "is the latest version! No upgrade necessary.";
            continue;
        }

        qInfo() << "Database" << cfg.m_name << "upgraded successfully!";

        /* PSUEDOCODE:
            // check database versions against template files
            auto current_db = getDatabaseVersion();
            checkVersionForUpgrade(current_db);

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
    }
}


/*!
 * @brief Retrieve database version
 * @param[in] Config for database file
 * @returns integer representing database version number
 */
TableSchema getDBVersion(const DatabaseConfig &cfg)
{
    TableSchema result;
    result.m_db_name = cfg.m_name;

    // segs_db scope: wrap this section in brackets to prevent unwanted
    // removeDatabase() errors
    {
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(cfg.m_driver, cfg.m_db_path));

        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(cfg.m_db_path); // must be path
        segs_db.setHostName(cfg.m_host);
        segs_db.setPort(cfg.m_port.toInt());
        segs_db.setUserName(cfg.m_user);
        segs_db.setPassword(cfg.m_pass);

        if(!segs_db.open())
        {
            qWarning() << "Failed to open database" << cfg.m_db_path;
            return result;
        }

        if(!segs_db.driver()->hasFeature(QSqlDriver::Transactions))
        {
            qWarning() << segs_db.driverName() << "does not support Transactions";
            return result;
        }

        QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
        if(!query.exec(querytext))
        {
            qWarning() << "SQL_ERROR:" << query.lastError();
            qWarning() << "QUERY:" << query.executedQuery();
            return result;
        }

        while (query.next())
        {
            result.m_version = query.value(0).toInt();
            qCDebug(logDB) << "Database Version:" << result.m_db_name << result.m_version;
        }
        query.finish();
        segs_db.close();

    } // end segs_db scope to prevent removeDatabase errors

    // unload the database (this doesn't delete it)
    QSqlDatabase::removeDatabase(cfg.m_db_path);
    return result; // return complete TableSchema object
}


/*
 * upgradeAccountsDB for checking database version vs g_accounts_upgrade_hooks
 */
bool checkVersionAndUpgrade(const DatabaseConfig &cfg, const TableSchema &cur_version)
{
    bool updates_completed = false; // will flag true if successful
    int target_version = cur_version.m_version + 1; // set target version for this iteration

    // run through g_segs_upgrade_hooks and check for upgrades
    // that are higher than our current DB version. Run them in order.
    for(const UpgradeHook &hook : g_segs_upgrade_hooks)
    {
        // g_defined_upgrade_hooks contains both databases, skip
        // databases that don't match the one we're currently checking
        if(hook.m_table_schema.m_db_name != cur_version.m_db_name)
        {
            qCDebug(logDB) << "We're currently looking for the other database, skipping to the next one."
                           << hook.m_table_schema.m_db_name << cur_version.m_db_name;
            continue;
        }

        // skip upgrades beneath the current db version
        if(hook.m_table_schema.m_version < target_version)
        {
            qCDebug(logDB) << "Upgrade hook is beneath current database version. Skip to the next one.";
            continue;
        }

        if(hook.m_table_schema.m_version == target_version)
        {
            qInfo() << "Upgrading database" << cfg.m_name << "to version" << target_version;
            if(hook.m_handler(cfg))
            {
                updates_completed = true;
                ++target_version; // iterate new version, because we've updated the database
            }
        }
    }

    if(updates_completed)
        return true;

    // Otherwise, no updates done.
    return false;
}

/*!
 * @brief Run query_text on database from cfg
 * @param[in] Config for database file
 * @param[in] QString of query text (SQL Query format)
 * @returns boolean indicating success
 */
bool runQuery(const DatabaseConfig &cfg, const QString query_text)
{
    // open database
    // somehow run hook here
    // close database

    return true; // if successful
}
