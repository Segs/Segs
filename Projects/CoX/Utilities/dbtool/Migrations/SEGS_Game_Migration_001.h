/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Utilities/dbtool/DBMigrationStep.h"

// TODO: how to handle updates to segs vs segs_game database?
// different classes? SEGS_Game_Migration vs SEGS_Migration?
class SEGS_Migration_001 : public DBMigrationStep
{
    int m_target_version = 1;
    std::vector<TableSchema> m_table_schemas = {
        {'db_version',1,'2018-01-23 10:27:01'},
        {'table_versions',0,'2017-11-11 08:57:42'},
        {'accounts',0,'2017-11-11 08:57:43'},
        {'characters',2,'2018-01-23 10:16:27'},
        {'costume',0,'2017-11-11 08:57:43'},
        {'progress',0,'2017-11-11 08:57:43'},
        {'supergroups',0,'2018-01-23 10:16:43'},
    };

    bool execute(DBConnection &db) override
    {
        qInfo() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

        // update database table schemas here
        // first let's add the supergroups table
        db.m_query->prepare("CREATE TABLE `supergroups` ("
                            "`id`	INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "`supergroup_id` INTEGER UNIQUE, "
                            "`sg_name` TEXT NOT NULL, "
                            "`sg_motto` TEXT, "
                            "`sg_motd` TEXT, "
                            "`sg_rank_names` BLOB, "
                            "`sg_rank_perms` BLOB, "
                            "`sg_emblem` BLOB, "
                            "`sg_colors` BLOB, "
                            "`sg_members` BLOB "
                            ")");

        if(!db.m_query->exec())
            return false;

        // second: add the chardata column to the characters table
        db.m_query->prepare("ALTER TABLE 'characters' ADD 'chardata' BLOB");
        if(!db.m_query->exec())
            return false;

        // third: copy the values from columns we plan to delete
        db.m_query->prepare("SELECT * FROM 'characters'");
        if(!db.m_query->exec())
            return false;

        while(db.m_query->next())
        {
            QVariantMap work_area;
            work_area["char_level"] = db.m_query->value(4);
            work_area["archetype"] = db.m_query->value(5);
            work_area["origin"] = db.m_query->value(6);
            work_area["current_map"] = db.m_query->value(8);
            work_area["last_costume_id"] = db.m_query->value(9);
            work_area["inf"] = db.m_query->value(13);
            work_area["xp"] = db.m_query->value(14);
            work_area["xpdebt"] = db.m_query->value(15);
            work_area["xppatrol"] = db.m_query->value(16);
            work_area["alignment"] = db.m_query->value(17);
            work_area["title"] = db.m_query->value(24);
            work_area["badgetitle"] = db.m_query->value(25);
            work_area["specialtitle"] = db.m_query->value(26);

            QJsonObject charObject;
            charObject.insert("Level", QJsonValue::fromVariant(work_area["char_level"]));
            charObject.insert("Archetype", QJsonValue::fromVariant(work_area["archetype"]));
            charObject.insert("Origin", QJsonValue::fromVariant(work_area["origin"]));
            charObject.insert("CurrentMap", QJsonValue::fromVariant(work_area["current_map"]));
            charObject.insert("LastCostumeId", QJsonValue::fromVariant(work_area["last_costume_id"]));
            charObject.insert("Influence", QJsonValue::fromVariant(work_area["inf"]));
            charObject.insert("XP", QJsonValue::fromVariant(work_area["xp"]));
            charObject.insert("XPDebt", QJsonValue::fromVariant(work_area["xpdebt"]));
            charObject.insert("XPPatrol", QJsonValue::fromVariant(work_area["xppatrol"]));
            charObject.insert("Alignment", QJsonValue::fromVariant(work_area["alignment"]));

            QJsonArray titlesArray;
            titlesArray.push_back(QJsonValue::fromVariant(work_area["title"]));
            titlesArray.push_back(QJsonValue::fromVariant(work_area["badgetitle"]));
            titlesArray.push_back(QJsonValue::fromVariant(work_area["specialtitle"]));
            charObject.insert("Title", titlesArray);

            QJsonDocument doc(charObject);
            qDebug().noquote() << doc.toJson();

            // fourth: move values to new location (chardata)
            QString querytext = QString("UPDATE characters SET charadata='%1'").arg(doc.toJson());
            if(!db.m_query->exec(querytext))
                return false;
        }

        // fifth: delete columns from characters table
        // 'char_level', `archetype`, `origin`, `current_map`, `last_costume_id`,
        // `inf`, `xp`, `xpdebt`, `xppatrol`, `alignment`, `title`,
        // `badgetitle`, `specialtitle`
        db.m_query->prepare("ALTER TABLE characters DROP COLUMN "
                            "char_level, archetype, origin, current_map, "
                            "last_costume_id, inf, xp, xpdebt, xppatrol, "
                            "alignment, title, badgetitle, specialtitle");
        if(!db.m_query->exec())
            return false;


        return true;
    }
};
