/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "DatabaseConfig.h"
#include "DBConnection.h"
#include "DBMigrationStep.h"
#include "Components/Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

// Must declare this method to populate MigrationStep vector
extern void register_migrations(std::vector<DBMigrationStep *> &target);

/*!
 * @brief Run database upgrades
 * @returns void
 */
void DBConnection::runUpgrades()
{
    qInfo().noquote() << QString("Checking for upgrades to database %1...").arg(getName());
    // register migrations to a vector that we can loop over
    std::vector<DBMigrationStep *> migrations_to_run;
    register_migrations(migrations_to_run);

    // get starting DB Version once to avoid multiple queries
    int start_version = getDBVersion();

    // check for the latest version
    int final_version = 0;
    for(DBMigrationStep *step : migrations_to_run)
    {
        if(getName() != step->getName())
            continue;

        qCDebug(logMigration) << "Checking migration step version:" << step->getName() << step->getTargetVersion();

        if(step->getTargetVersion() > final_version)
            final_version = step->getTargetVersion();
    }

    qCDebug(logMigration) << "Final migration version:" << final_version;

    // check database version against schema in default folder
    if(start_version >= final_version)
    {
        qInfo().noquote() << QString("Database %1 is already the latest version! No upgrade necessary.").arg(getName());
        return;
    }

    // set cur_version to starting version, it will be incremented as we update
    int cur_version = start_version;

    // run through migrations_to_run and check for upgrades
    // that are higher than our current DB version. Run them in order.
    for(DBMigrationStep *step : migrations_to_run)
    {
        if(!step->canRun(this, cur_version))
            continue; // can't run? skip this migration step.

        qInfo().noquote() << QString("Running %1 database migration from version %2 to %3...")
                   .arg(getName())
                   .arg(cur_version)
                   .arg(step->getTargetVersion());

        if(!step->execute(this))
        {
            qInfo().noquote() << QString("Database %1 failed to update to version %2! Rolling back changes.")
                       .arg(getName())
                       .arg(step->getTargetVersion());

            qWarning() << "Execute query failed:" << m_query->lastError();
            qWarning() << "QUERY:" << m_query->executedQuery();
            m_db->rollback();
            return; // execution failed, cancel update
        }

        if(!step->cleanup(this))
            return; // cleanup failed, cancel update

        cur_version++; // increment current version for next run
    }

    // check if all updates succeeded in running
    if(getDBVersion() < final_version)
    {
        qWarning() << "UPDATE FAILED! Rolling back database.";
        m_db->rollback();
        return;
    }

    // finally: attempt to commit changes
    if(!m_db->commit())
    {
        qWarning() << "Database update failed:" << m_query->lastError();
        qWarning() << "QUERY:" << m_query->executedQuery();
        m_db->rollback();
        return;
    }

    qInfo().noquote() << QString("SUCCESS! Database %1 upgraded to %2!")
               .arg(getName())
               .arg(getDBVersion()); // use getDBVersion here so we know it worked
    return;
}


/*!
 * @brief Retrieve database version
 * @returns integer representing database version number
 */
int DBConnection::getDBVersion()
{
    int version = 0;
    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!m_query->exec(querytext))
    {
        qWarning() << "SQL_ERROR:" << m_query->lastError();
        qWarning() << "QUERY:" << m_query->executedQuery();
        return version;
    }
    while(m_query->next())
    {
        version = m_query->value(0).toInt();
    }

    qCDebug(logDB) << "Fetching database version:" << getName() << version;
    return version;
}

int getFinalMigrationVersion(std::vector<DBMigrationStep *> &migrations, const QString &db_name)
{
    int final_version = 0;
    for(DBMigrationStep *step : migrations)
    {
        if(db_name != step->getName())
            continue;

        if(step->getTargetVersion() < final_version)
            final_version = step->getTargetVersion();
    }

    return final_version;
}

bool DBConnection::updateTableVersions(const DBSchemas &table_schemas)
{
    for(const TableSchema &table : table_schemas)
    {
        QString query_txt = QString("UPDATE table_versions SET version = '%1', last_update = '%2' WHERE table_name = '%3';")
                .arg(table.m_version)
                .arg(table.m_last_updated)
                .arg(table.m_table_name);

        m_query->prepare(query_txt);
        if(!m_query->exec())
            return false;

        qCDebug(logMigration) << "Updating Version:" << table.m_table_name << table.m_version << table.m_last_updated;
    }

    return true;
}

//! @}
