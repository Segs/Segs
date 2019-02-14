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

DBMigrationStep::DBMigrationStep(DBConnection &db)
{
    if(!canRun(db))
    {
        qInfo() << "Cannot run migration step" << getVersion();
        return;
    }

    if(!execute(db))
    {
        qWarning() << "Failed to execute database upgrades to version"
                   << getVersion() << "! Rolling back database.";
        db.m_db.rollback();
        return;
    }

    if(!cleanup(db))
        return;
}

bool DBMigrationStep::cleanup(DBConnection &db)
{
    // attempt to update table versions
    if(!db.updateTableVersions(m_table_schemas))
    {
        qWarning() << "Failed to update database schema versions! Rolling back database.";
        db.m_db.rollback();
        return false;
    }

    // attempt to commit changes
    if(!db.m_db.commit())
    {
        qWarning() << "Commit failed:" << db.m_query->lastError();
        qWarning() << "QUERY:" << db.m_query->executedQuery();
        db.m_db.rollback();
        return false;
    }

    return true; // if successful
}
