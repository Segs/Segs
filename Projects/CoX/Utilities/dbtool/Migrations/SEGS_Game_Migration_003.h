/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

class SEGS_Game_Migration_003 : public DBMigrationStep
{
private:
    int m_target_version = 3;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version",3,"2018-01-23 10:27:01"},
        {"table_versions",0,"2017-11-11 08:57:42"},
        {"accounts",0,"2017-11-11 08:57:43"},
        {"characters",2,"2018-01-23 10:16:27"},
        {"costume",0,"2017-11-11 08:57:43"},
        {"progress",0,"2017-11-11 08:57:43"},
        {"supergroups",0,"2018-01-23 10:16:43"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        bool success = false;

        // update database table schemas here
        // update cereal blobs once schemas are correct
        if(!success)
           return false;

        return true;
    }
};
