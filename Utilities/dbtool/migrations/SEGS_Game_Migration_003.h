/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_003 : public DBMigrationStep
{
private:
    int m_target_version = 3;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 3, "2018-01-28 10:27:01"},
        {"characters", 4, "2018-01-28 10:16:27"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // update database table schemas here
        // first: select the data from characters table
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        // second column copy data over to entity data blob
        while(db->m_query->next())
        {
            QJsonObject ent_obj = db->loadBlob("entitydata");

            // third update cereal_class_version
            ent_obj["cereal_class_version"] = 2; // set to 2

            // fourth: move values to new location (table column to entdata)
            QJsonArray pos_arr;
            pos_arr.push_back(QJsonValue::fromVariant(db->m_query->value("posx")));
            pos_arr.push_back(QJsonValue::fromVariant(db->m_query->value("posy")));
            pos_arr.push_back(QJsonValue::fromVariant(db->m_query->value("posz")));
            ent_obj.insert("Position", pos_arr);

            QJsonArray orient_arr;
            orient_arr.push_back(QJsonValue::fromVariant(db->m_query->value("orientp")));
            orient_arr.push_back(QJsonValue::fromVariant(db->m_query->value("orienty")));
            orient_arr.push_back(QJsonValue::fromVariant(db->m_query->value("orientr")));
            ent_obj.insert("Orientation", orient_arr);

            QString entdoc = db->saveBlob(ent_obj);
            qCDebug(logMigration).noquote() << entdoc;  // print output for debug

            QString querytext = QString("UPDATE characters SET entitydata='%1'")
                    .arg(entdoc);
            if(!db->m_query->exec(querytext))
                return false;
        }

        // finally: delete several columns from characters table
        // these are now stored in entitydata
        // posx, posy, posz, orientp, orienty, orientr
        QStringList cols_to_drop = {
            "posx", "posy", "posz", "orientp", "orienty", "orientr"
        };
        db->deleteColumns(QStringLiteral("characters"), cols_to_drop);

        // we're done, return true
        return true;
    }
};
