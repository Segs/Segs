/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DatabaseConfig.h"
#include "DBConnection.h"
#include "DBMigrationStep.h"

#include "Logging.h"

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
    qInfo() << "Upgrading database" << getName();
    // register migrations to a vector that we can loop over
    std::vector<DBMigrationStep *> migrations_to_run;
    register_migrations(migrations_to_run);

    // check for the latest version
    int final_version = 0;
    for(DBMigrationStep *step : migrations_to_run)
    {
        if(getName() != step->getName())
            continue;

        if(step->getTargetVersion() < final_version)
            final_version = step->getTargetVersion();
    }

    // check database version against schema in default folder
    if(getDBVersion() >= final_version)
    {
        qInfo() << QString("Database %1 is already the latest version! No upgrade necessary.").arg(getName());
        //return;
    }

    // run through migrations_to_run and check for upgrades
    // that are higher than our current DB version. Run them in order.
    for(DBMigrationStep *step : migrations_to_run)
    {
        if(!step->canRun(this))
            continue; // can't run? skip it.

        qInfo() << QString("Running %1 database migration from version %2 to %3")
                   .arg(getName())
                   .arg(getDBVersion())
                   .arg(step->getTargetVersion());

        if(!step->execute(this))
        {
            qInfo() << QString("Database %1 failed to update to version %2! Rolling back changes.")
                       .arg(getName())
                       .arg(getDBVersion()+1);

            m_db->rollback();
            return; // execution failed, cancel update
        }

        if(!step->cleanup(this))
            return; // cleanup failed, cancel update
    }

    qInfo() << QString("Database %1 upgraded successfully to %2!")
               .arg(getName())
               .arg(getDBVersion());
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
        qCDebug(logDB) << "Database Version:" << getName() << version;
    }

    return version;
}

int getFinalMigrationVersion(std::vector<DBMigrationStep *> &migrations, QString &db_name)
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

bool DBConnection::updateTableVersions(DBSchemas &table_schemas)
{
    Q_UNUSED(table_schemas);
    return false;
}
