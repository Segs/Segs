/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "UpgradeHooks.h"
#include "DatabaseConfig.h"

#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>


// handlers for segs db upgrades
// please add new handlers in numerical order
bool upgradeHandler_accounts_001(const DBConnection *db, QSqlQuery &query);

// handlers for segs_game db upgrades
// please add new handlers in numerical order
bool upgradeHandler_game_001(const DBConnection *db, QSqlQuery &query);
bool upgradeHandler_game_002(const DBConnection *db, QSqlQuery &query);

// upgrade hook vector
// please add new handlers in numberical order
const std::vector<UpgradeHook> g_segs_upgrade_hooks = {
    /* segs database changes */
    {{"segs",   1},         upgradeHandler_accounts_001},
    /* segs_game database Changes */
    {{"segs_game",  10},    upgradeHandler_game_001},
    {{"segs_game",  11},    upgradeHandler_game_002},
};

/*
 * segs db upgradeHandlers
 * please add handlers in numerical order
 */
bool upgradeHandler_accounts_001(const DBConnection *db, QSqlQuery &query)
{
    qWarning() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!query.exec(querytext))
        return false;

    while(query.next())
    {
        int cur_version = query.value(0).toInt();

        querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query.exec(querytext))
            return false;
    }

    return true; // if successful
}

/*
 * segs_game db upgradeHandlers
 * please add handlers in numerical order
 */
bool upgradeHandler_game_001(const DBConnection *db, QSqlQuery &query)
{
    qWarning() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!query.exec(querytext))
        return false;

    while(query.next())
    {
        int cur_version = query.value(0).toInt();

        querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query.exec(querytext))
            return false;
    }

    return true; // if successful
}

bool upgradeHandler_game_002(const DBConnection *db, QSqlQuery &query)
{
    qWarning() << "PERFORMING UPGRADE 002 on" << db->m_config.m_db_path;

    QString querytext = "SELECT ver3sion FROM table_versions WHERE table_name='db_version'";
    if(!query.exec(querytext))
        return false;

    while(query.next())
    {
        int cur_version = query.value(0).toInt();

        querytext = QString("UPDATE table_versions SET ver3sion='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query.exec(querytext))
            return false;
    }

    return true; // if successful
}
