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
bool upgradeHandler_accounts_001(const DatabaseConfig &cfg);

// handlers for segs_game db upgrades
// please add new handlers in numerical order
bool upgradeHandler_game_001(const DatabaseConfig &cfg);
bool upgradeHandler_game_002(const DatabaseConfig &cfg);

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
bool upgradeHandler_accounts_001(const DatabaseConfig &cfg)
{
    qWarning() << "DO UPGRADE 001 on" << cfg.m_db_path;
    int cur_version = 0;

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
            return false;
        }

        if(!segs_db.driver()->hasFeature(QSqlDriver::Transactions))
        {
            qWarning() << segs_db.driverName() << "does not support Transactions";
            return false;
        }

        QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
        if(!query.exec(querytext))
        {
            qWarning() << "SQL_ERROR:" << query.lastError();
            qWarning() << "QUERY:" << query.executedQuery();
            return false;
        }

        while (query.next())
        {
            cur_version = query.value(0).toInt();
        }

        querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query.exec(querytext))
        {
            qWarning() << "SQL_ERROR:" << query.lastError();
            qWarning() << "QUERY:" << query.executedQuery();
            return false;
        }

        qInfo() << QString("Upgrading Database %1, from version %2 to version %3.")
                 .arg(cfg.m_name)
                 .arg(cur_version)
                 .arg(cur_version+1);

        query.finish();
        segs_db.close();

    } // end segs_db scope to prevent removeDatabase errors

    // unload the database (this doesn't delete it)
    QSqlDatabase::removeDatabase(cfg.m_db_path);

    return true; // if successful
}

/*
 * segs_game db upgradeHandlers
 * please add handlers in numerical order
 */
bool upgradeHandler_game_001(const DatabaseConfig &cfg)
{
    qWarning() << "DO UPGRADE 001 on" << cfg.m_db_path;

    /*
    result.m_version = query.value(0).toInt();

    querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(result.m_version + 1);

    qInfo << "Upgrading Database" << result.m_db_name
          << "from version" << result.m_version
          << "to version" << result.m_version+1;
    */

    return true; // if successful
}

bool upgradeHandler_game_002(const DatabaseConfig &cfg)
{
    qWarning() << "DO UPGRADE 002 on" << cfg.m_db_path;
    return true; // if successful
}
