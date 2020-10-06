/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_010 : public DBMigrationStep
{
private:
    int m_target_version = 10;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 10, "2019-04-28 22:56:43"},
        {"characters", 12, "2019-04-28 22:56:43"},
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

        // Add player progress to characters table column player_data
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        while(db->m_query->next())
        {
            QJsonObject player_obj = db->loadBlob("player_data");
            QJsonObject progress_obj;
            QJsonObject mapcells_obj;

            player_obj["cereal_class_version"] = 4; // set to 4

            progress_obj.insert("cereal_class_version", 1);
            progress_obj.insert("VisibleMapCells", mapcells_obj);
            player_obj.insert("Progress", progress_obj);

            QString player_data_json = db->saveBlob(player_obj);
            qCDebug(logMigration).noquote() << "progress:" << player_data_json; // print output for debug

            QString querytext = QString("UPDATE characters SET player_data='%1'")
                    .arg(player_data_json);
            if(!db->runQuery(querytext))
                return false;
        }

        return true;
    }
};
