/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

// This migration will only work if db_version 0 is manually added
// to the segs database.
class SEGS_Migration_001 : public DBMigrationStep
{
    int m_target_version = 1;
    std::vector<TableSchema> m_table_schemas = {
        {'db_version',0,'2018-01-06 16:27:58'},
        {'table_versions',0,'2017-11-11 08:55:54'},
        {'accounts',1,'2018-01-06 11:18:01'},
        {'game_servers',0,'2017-11-11 09:12:37'},
        {'bans',0,'2017-11-11 09:12:37'},
    };

    bool execute(DBConnection &db) override
    {
        qInfo() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

        // update database table schemas here
        QStringList queries = {
            "ALTER TABLE 'accounts' ADD 'salt' BLOB",
            // NOTE: there's a typo in db_version version number, which was set to 0 in the update
            "UPDATE 'table_versions' SET version='0', last_update='2018-01-06 16:27:58' WHERE table_name='db_version'",
            "UPDATE 'table_versions' SET version='1', last_update='2018-01-06 11:18:01' WHERE table_name='accounts'",
        };

        for(QString &q : queries)
        {
            if(!query->exec(q))
                return false;
        }

        return true;
    }
};
