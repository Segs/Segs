/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

class SEGS_Game_Migration_009 : public DBMigrationStep
{
private:
    int m_target_version = 9;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 9, "2018-10-22 22:56:43"},
        {"characters", 9, "2018-10-22 22:56:43"},
        {"supergroups", 2, "2018-10-22 22:56:43"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // Add new blobs and remove unused sg columns
        QStringList queries = {
            "ALTER TABLE 'characters' ADD 'costume_data' BLOB", // add new costume_data blob to characters table
            "ALTER TABLE 'supergroups' ADD 'sg_data' BLOB",     // add new sg_data blob to supergroups table
            "ALTER TABLE characters DROP COLUMN sg_motto, sg_motd, sg_rank_names, sg_rank_perms, sg_emblem, sg_colors",
        };

        for(auto &q : queries)
        {
            db->m_query->prepare(q);
            if(!db->m_query->exec())
                return false;
        }



        // we're done, return true
        return true;
    }
};
