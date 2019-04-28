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
        qCDebug(logMigration).noquote() << QString("PERFORMING UPGRADE %1 on %2")
                          .arg(getTargetVersion())
                          .arg(db->getName());

        // select existing costumes from costume table
        // we only knew how to save one costume per character, so we
        // can make some assumptions about costume index
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        while(db->m_query->next())
        {
            QVariantMap char_obj = db->loadBlob("chardata");
            char_obj["LastOnline"] = "test value";
            char_obj["Level"] = 49;
            char_obj["HasTitles"] = true;
            char_obj["ThePrefix"] = true;
            char_obj["Titles"] = QStringList({"Title1","Title2","Title3"});

            QString chardoc = db->saveBlob(char_obj);
            qCDebug(logMigration).noquote() << chardoc; // print output for debug
        }

        // update database table schemas here
        // update cereal blobs once schemas are correct

        return true;
    }
};
