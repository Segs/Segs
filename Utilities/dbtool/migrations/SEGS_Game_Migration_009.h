/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_009 : public DBMigrationStep
{
private:
    int m_target_version = 9;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 9, "2018-10-22 22:56:43"},
        {"characters", 12, "2019-04-28 22:56:43"},
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

        QStringList sg_cols_to_drop = {
            "sg_motto", "sg_motd", "sg_rank_names", "sg_rank_perms",
            "sg_emblem", "sg_colors"
        };
        db->deleteColumns(QStringLiteral("supergroups"), sg_cols_to_drop);

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
            costume_obj.insert("CharacterID", QJsonValue::fromVariant(char_id));

            // get character data for character that owns costume
            QSqlQuery char_query(*db->m_db);
            QString querytext = QString("SELECT * FROM 'characters' WHERE id=%1").arg(char_id.toUInt());
            char_query.prepare(querytext);
            if(!char_query.exec())
                return false;

            while(char_query.next())
            {
                costume_obj.insert("Height", char_query.value("height").toFloat());
                costume_obj.insert("Physique", char_query.value("physique").toFloat());
                costume_obj.insert("BodyType", QJsonValue::fromVariant(char_query.value("bodytype")));
            }

            costume_obj.insert("CostumeIdx", QJsonValue::fromVariant(db->m_query->value("costume_index")));
            costume_obj.insert("SkinColor", QJsonValue::fromVariant(db->m_query->value("skin_color")));
            costume_obj.insert("SendFullCostume", true);
            costume_obj.insert("NumParts", 15); // all player "primary" costumes are 15

            // parts object can be copied wholesale, but we need to strip the object
            // and add cereal's version value, also we changed how we store value indexes
            // and we need to update them 0-15
            // cereal objects are wrapped in key 'value0'
            QJsonArray parts_arr = db->loadBlob("parts")["value0"].toArray();
            for(int i = 0; i < parts_arr.size(); ++i)
            {
                QJsonObject part_obj = parts_arr.at(i).toObject();
                if(i == 0) // cereal requires class version in first object ONLY.
                    part_obj.insert("cereal_class_version", 1);

                part_obj["value0"] = i;
                parts_arr.replace(i,part_obj);
            }
            costume_obj.insert("Parts", parts_arr); // save it back here

            // costumes are now saved as a vector of objects, add to jsonarray
            db->prepareCerealArray(costume_obj);
            QString costumes_json = db->saveBlob(costume_obj);
            //qCDebug(logMigration).noquote() << "costumes:" << costumes_json; // print output for debug

            querytext = QString("UPDATE characters SET costume_data='%1'")
                    .arg(costumes_json);
            if(!db->runQuery(querytext))
                return false;
        }

        // these have been moved into the costume blob
        QStringList char_cols_to_drop = {
            "bodytype", "physique", "height"
        };
        db->deleteColumns(QStringLiteral("characters"), char_cols_to_drop);

        QString drop_qry = "DROP TABLE costume";
        if(!db->runQuery(drop_qry))
            return false;

        // We never iterated our database version for changes to the character data
        // table v9-v12, so we update those here
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        while(db->m_query->next())
        {
            QJsonObject char_obj = db->loadBlob("chardata");
            QJsonObject ent_obj = db->loadBlob("entitydata");
            QJsonObject player_obj = db->loadBlob("player_data");

            // iterate cereal class versions
            char_obj["cereal_class_version"] = 12; // set to 12
            player_obj["cereal_class_version"] = 3; // set to 3

            // set MapIdx to new value because mapidxs changed and 0 doesn't work
            ent_obj["MapIdx"] = 24; // Outbreak
            QString ent_data_json = db->saveBlob(ent_obj);
            //qCDebug(logMigration).noquote() << "entitydata:" << ent_data_json; // print output for debug

            // Transfer KnownContacts over from chardata and remove field
            player_obj.insert("KnownContacts", char_obj["KnownContact"].toArray());
            char_obj.remove("KnownContact");
            QString char_data_json = db->saveBlob(char_obj);
            //qCDebug(logMigration).noquote() << "chardata:" << char_data_json; // print output for debug

            // Add these arrays. They can be empty
            QJsonArray tasks_arr;
            player_obj.insert("Tasks", tasks_arr);
            QJsonArray clues_arr;
            player_obj.insert("Clue", clues_arr); // note: Keyword `Clue` is singular here
            QJsonArray souvenirs_arr;
            player_obj.insert("Souvenirs", souvenirs_arr);

            // Statistics obj requires class version info
            QJsonObject statistics_obj;
            statistics_obj.insert("cereal_class_version", 2); // we jump to 2 because no one iterated the prior version
            QJsonObject hidenseek_obj;
            hidenseek_obj.insert("cereal_class_version", 1);
            hidenseek_obj.insert("Count", 0);
            statistics_obj.insert("HideAndSeek", hidenseek_obj);
            QJsonArray relay_arr;
            statistics_obj.insert("RelayRaces", relay_arr);
            QJsonArray hunts_arr;
            statistics_obj.insert("Hunts", hunts_arr);
            // save completed statistics_obj to player_obj
            player_obj.insert("Statistics", statistics_obj);

            QString player_data_json = db->saveBlob(player_obj);
            //qCDebug(logMigration).noquote() << "player_data:" << player_data_json; // print output for debug

            QString querytext = QString("UPDATE characters SET chardata='%1', entitydata='%2', player_data='%3'")
                    .arg(char_data_json)
                    .arg(ent_data_json)
                    .arg(player_data_json);

            if(!db->runQuery(querytext))
                return false;
        }

        // we're done, return true for success
        return true;
    }
};
