/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBConnection.h"

class DBMigrationStep
{
    DBMigrationStep(DBConnection &db);
public:
    int m_target_version = 0;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", m_target_version}
    };

    int getVersion() const { return m_target_version; }
    bool canRun(DBConnection &db) const { return db.checkTableVersions(m_table_schemas); }
    bool virtual execute(DBConnection &/*db*/) { return false; }
    bool cleanup(DBConnection &db);
};
