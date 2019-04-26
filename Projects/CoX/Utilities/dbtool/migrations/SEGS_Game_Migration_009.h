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
        };
        if(!db->runQueries(queries))
            return false;

        QStringList cols_to_drop = {
            "sg_motto", "sg_motd", "sg_rank_names", "sg_rank_perms",
            "sg_emblem", "sg_colors"
        };
        db->deleteColumns(QStringLiteral("supergroups"), cols_to_drop);

        // select existing costumes from costume table
        // we only knew how to save one costume per character, so we
        // can make some assumptions about costume index
        db->m_query->prepare("SELECT * FROM 'costume'");
        if(!db->m_query->exec())
            return false;

        // copy costume data over to character costume blob
        while(db->m_query->next())
        {
            QVariant char_id = db->m_query->value("character_id");

            QJsonObject costume_obj;
            costume_obj.insert("cereal_class_version", 1);
            costume_obj.insert("CharacterID", char_id.toJsonValue());

            // get character data for character that owns costume
            QSqlQuery char_query(*db->m_db);
            QString querytext = QString("SELECT * FROM 'characters' WHERE id=%1").arg(char_id.toInt());
            char_query.prepare(querytext);
            if(!char_query.exec())
                return false;

            while(char_query.next())
            {
                costume_obj.insert("Height", char_query.value("height").toJsonValue());
                costume_obj.insert("Physique", char_query.value("physique").toJsonValue());
                costume_obj.insert("BodyType", char_query.value("bodytype").toJsonValue());
            }

            costume_obj.insert("CostumeIdx", db->m_query->value("costume_index").toJsonValue());
            costume_obj.insert("SkinColor", db->m_query->value("skin_color").toJsonValue());
            costume_obj.insert("SendFullCostume", true);

            // parts object can be copied wholesale
            QJsonArray parts_arr = db->m_query->value("parts").toJsonArray();
            costume_obj.insert("NumParts", 15); // all player "primary" costumes are 15
            costume_obj.insert("Parts", parts_arr); // cereal objects are wrapped in key 'value0'

            db->prepareBlob(costume_obj);
            QJsonDocument costumedoc(costume_obj);
            qCDebug(logMigration).noquote() << costumedoc.toJson(); // print output for debug

            querytext = QString("UPDATE characters SET costume_data='%1'")
                    .arg(QString(costumedoc.toJson()));
            if(!db->m_query->exec(querytext))
                return false;
        }

        QString drop_qry = "DROP TABLE costume";
        return db->runQuery(drop_qry);
    }
};
