/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_008 : public DBMigrationStep
{
private:
    int m_target_version = 8;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 8, "2018-05-03 17:52:33"},
        {"emails", 0, "2018-09-23 08:00:00"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // This update adds the emails table
        db->m_query->prepare("INSERT INTO table_versions VALUES(8,'emails',0,'2018-09-23 08:00:00')");
        if(!db->m_query->exec())
            return false;

        QString email_table = QStringLiteral("CREATE TABLE 'emails'("
            "`id`	INTEGER PRIMARY KEY AUTOINCREMENT,"
            "`sender_id`	INTEGER NOT NULL,"
            "`recipient_id` INTEGER NOT NULL,"
            "`email_data` BLOB,"
            "FOREIGN KEY(`sender_id`) REFERENCES characters ( id ) ON DELETE CASCADE,"
            "FOREIGN KEY(`recipient_id`) REFERENCES characters ( id ) ON DELETE CASCADE"
            ")");

        if(!db->runQuery(email_table))
            return false;

        return true;
    }
};
