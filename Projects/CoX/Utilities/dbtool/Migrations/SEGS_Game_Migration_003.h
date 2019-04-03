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
            QJsonObject char_obj = db->m_query->value("chardata").toJsonObject();
            db->loadBlob(char_obj);
            //charObject["value0"].insert("LastOnline", ""); // cereal objects are wrapped in key 'value0'
            db->saveBlob(char_obj);
            QJsonDocument chardoc(char_obj);

            QJsonObject ent_obj = db->m_query->value("entitydata").toJsonObject();
            db->loadBlob(ent_obj);
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

            db->saveBlob(ent_obj);
            QJsonDocument entdoc(ent_obj);
            //qDebug().noquote() << chardoc.toJson(); // print output for debug
            //qDebug().noquote() << entdoc.toJson();  // print output for debug

            QString querytext = QString("UPDATE characters SET chardata='%1', entitydata='%2'")
                    .arg(QString(chardoc.toJson()))
                    .arg(QString(entdoc.toJson()));
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
