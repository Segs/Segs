/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "UpgradeHooks.h"
#include "DatabaseConfig.h"
#include "DBConnection.h"

#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


/*!
 * @brief Run database upgrades
 * @returns void
 */
void DBConnection::runUpgrades()
{
    qInfo() << "Upgrading database" << getName();
    // check database version against schema in default folder
    if(!checkVersionAndUpgrade(getDBVersion()))
    {
        qInfo() << "Database" << m_config.m_name << "is the latest version! No upgrade necessary.";
        return;
    }

    qInfo() << "Database" << m_config.m_name << "upgraded successfully!";

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


/*!
 * @brief Retrieve database version
 * @returns integer representing database version number
 */
TableSchema DBConnection::getDBVersion()
{
    TableSchema result;
    result.m_db_name = getName();

    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!m_query->exec(querytext))
    {
        qWarning() << "SQL_ERROR:" << m_query->lastError();
        qWarning() << "QUERY:" << m_query->executedQuery();
        return result;
    }
    while(m_query->next())
    {
        result.m_version = m_query->value(0).toInt();
        qCDebug(logDB) << "Database Version:" << result.m_db_name << result.m_version;
    }

    return result; // return complete TableSchema object
}


/*!
 * @brief Check version and run upgrade
 * @param[in] Config for database file
 * @returns integer representing database version number
 */
bool DBConnection::checkVersionAndUpgrade(const TableSchema &cur_version)
{
    int target_version = cur_version.m_version + 1; // set target version for this iteration
    std::vector<UpgradeHook> hooks_to_run;

    // run through g_segs_upgrade_hooks and check for upgrades
    // that are higher than our current DB version. Run them in order.
    for(const UpgradeHook &hook : g_segs_upgrade_hooks)
    {
        // g_segs_upgrade_hooks contains both databases, skip
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
            hooks_to_run.push_back(hook);
            ++target_version; // iterate new version, because we've updated the database
        }
    }

    // run upgrades and return false if failed
    if(!runUpgradeHooks(hooks_to_run))
        return false;

    // Otherwise, success!
    return true;
}

/*!
 * @brief Run hooks on database from cfg
 * @param[in] Config for database file
 * @param[in] Vector of UpgradeHooks
 * @returns boolean indicating success
 */
bool DBConnection::runUpgradeHooks(const std::vector<UpgradeHook> &hooks_to_run)
{
    // run all hooks in order, under same transaction
    for(const UpgradeHook &hook : hooks_to_run)
    {
        // hooks are in order already
        if(!hook.m_handler(this, m_query))
            break;

        qInfo() << "Upgrading database" << getName() << "to version" << hook.m_table_schema.m_version;
    }

    // close database
    if(!m_db.commit())
    {
        qWarning() << "Commit failed:" << m_query->lastError();
        qWarning() << "QUERY:" << m_query->executedQuery();
        m_db.rollback();
        return false;
    }

    return true; // if successful
}
