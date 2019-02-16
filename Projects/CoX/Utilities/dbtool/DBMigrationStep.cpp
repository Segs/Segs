/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBMigrationStep.h"
#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

DBMigrationStep::DBMigrationStep()
{
}

bool DBMigrationStep::canRun(DBConnection *db)
{
    // migrations_to_run may contain both databases, skip
    // databases that don't match the one we're currently checking
    if(getName() != db->getName())
    {
        qCDebug(logDB) << QString("We're currently looking for %1 database, but found %2. Skipping to the next migration in the list.")
                          .arg(getName(), db->getName());
        return false;
    }

    // skip migrations with a target version beneath the current db version
    if(getTargetVersion() <= db->getDBVersion())
    {
        qCDebug(logDB) << "Migration step is beneath current database version. Skipping to the next one.";
        return false;
    }

    // if current database version is one less than the target version, run it.
    if(db->getDBVersion()+1 == getTargetVersion())
        return true;

    qCDebug(logDB) << QString("Cannot run migration step %1 on %2 database.").arg(getTargetVersion()).arg(db->getName());
    return false;
}

bool DBMigrationStep::cleanup(DBConnection *db)
{
    // attempt to update table versions
    if(!db->updateTableVersions(m_table_schemas))
    {
        qWarning() << "Failed to update database schema versions! Rolling back database.";
        db->m_db->rollback();
        return false;
    }

    // attempt to commit changes
    if(!db->m_db->commit())
    {
        qWarning() << "Commit failed:" << db->m_query->lastError();
        qWarning() << "QUERY:" << db->m_query->executedQuery();
        db->m_db->rollback();
        return false;
    }

    return true; // if successful
}
