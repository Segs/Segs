/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Migration_001 : public DBMigrationStep
{
private:
    int m_target_version = 1;
    QString m_name = SEGS_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 1, "2019-02-17 03:11:58"},
        {"table_versions", 0, "2017-11-11 08:55:54"},
        {"accounts", 1, "2018-01-06 11:18:01"},
        {"game_servers", 0, "2017-11-11 09:12:37"},
        {"bans", 0, "2017-11-11 09:12:37"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // this migration is purely to synchronize db_version
        // with the sql script file and reflect past updates to accounts table
        // it will always return true, and will always succeed.

        qCDebug(logMigration).noquote() << QString("PERFORMING UPGRADE %1 on %2")
                          .arg(getTargetVersion())
                          .arg(db->getName());

        return true;
    }
};
