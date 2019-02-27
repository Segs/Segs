/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

class SEGS_Game_Migration_002 : public DBMigrationStep
{
private:
    int m_target_version = 2;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 2, "2018-01-25 10:27:01"},
        {"characters", 3, "2018-01-25 10:16:27"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // update database table schemas here
        // first: add the entitydata column to the characters table
        db->m_query->prepare("ALTER TABLE 'characters' ADD 'entitydata' BLOB");
        if(!db->m_query->exec())
            return false;

        // second: delete last_online column from characters table
        db->m_query->prepare("ALTER TABLE characters DROP COLUMN last_online");
        if(!db->m_query->exec())
            return false;

        // third: copy the values from columns we plan to delete
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        while(db->m_query->next())
        {
            QJsonObject char_obj = db->m_query->value("chardata").toJsonObject();
            db->loadBlob(char_obj);
            char_obj.insert("LastOnline", ""); // cereal objects are wrapped in key 'value0'
            db->saveBlob(char_obj);

            // fourth: move values to new location (chardata and entdata)
            QJsonObject ent_obj;
            ent_obj.insert("OriginIdx", 1);     // TODO: how to actually determine this?
            ent_obj.insert("ClassIdx", 1);      // TODO: how to actually determine this?
            ent_obj.insert("Type", 2);          // EntType == Player
            ent_obj.insert("Idx", 0);
            ent_obj.insert("dbID", 0);
            db->saveBlob(ent_obj);  // required by cereal

            QJsonDocument chardoc(char_obj);
            QJsonDocument entdoc(ent_obj);
            //qDebug().noquote() << chardoc.toJson(); // print output for debug
            //qDebug().noquote() << entdoc.toJson();  // print output for debug

            QString querytext = QString("UPDATE characters SET chardata='%1', entitydata='%2'")
                    .arg(QString(chardoc.toJson()))
                    .arg(QString(entdoc.toJson()));
            if(!db->m_query->exec(querytext))
                return false;
        }

        // we're done, return true
        return true;
    }
};
