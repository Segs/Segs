/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

class SEGS_Game_Migration_010 : public DBMigrationStep
{
private:
    int m_target_version = 10;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 10, "2018-01-23 10:27:01"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        qCDebug(logDB).noquote() << QString("PERFORMING UPGRADE %1 on %2")
                          .arg(getTargetVersion())
                          .arg(db->getName());

        // update database table schemas here
        // update cereal blobs once schemas are correct

        return true;
    }
};
