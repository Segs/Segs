/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

class SEGS_Migration_10 : public DBMigrationStep
{
    int m_target_version = 10;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", m_target_version},
        {"characters", 9},
        {"supergroups", 2}
    };

    bool execute(DBConnection &db) override
    {
        bool success = false;

        // update database table schemas here
        // update cereal blobs once schemas are correct
        if(!success)
           return false;

        return true;
    }
};
