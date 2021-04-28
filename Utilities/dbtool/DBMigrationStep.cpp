/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBMigrationStep.h"
#include "Components/Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

DBMigrationStep::DBMigrationStep()
{
}

bool DBMigrationStep::canRun(DBConnection *db, int cur_version)
{
    // migrations_to_run may contain both databases, skip
    // databases that don't match the one we're currently checking
    if(getName() != db->getName())
    {
        qCDebug(logMigration).noquote() << QString("We're currently looking for %1 database, but found %2. Skipping to the next migration in the list.")
                          .arg(getName(), db->getName());
        return false;
    }

    // skip migrations with a target version beneath the current db version
    if(getTargetVersion() <= cur_version)
    {
        qCDebug(logMigration).noquote() << QString("Migration step version %1 is beneath current database version %2. Skipping to the next one.")
                          .arg(getTargetVersion())
                          .arg(cur_version);
        return false;
    }

    // if current database version is one less than the target version, run it.
    if(getTargetVersion() == cur_version + 1)
        return true;

    qCDebug(logMigration).noquote() << QString("Cannot run migration step %1 on %2 database.").arg(getTargetVersion()).arg(db->getName());
    return false;
}

bool DBMigrationStep::cleanup(DBConnection *db)
{
    DBSchemas schemas = getTableVersions();
    // attempt to update table versions
    if(!db->updateTableVersions(schemas))
    {
        qWarning() << "Failed to update database schema versions! Rolling back database.";
        db->m_db->rollback();
        return false;
    }

    qCDebug(logMigration).noquote() << QString("Running commit on upgrade %1 on %2...")
                  .arg(getTargetVersion())
                  .arg(db->getName());
    return true; // successful
}
