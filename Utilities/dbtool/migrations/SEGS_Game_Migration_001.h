/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_001 : public DBMigrationStep
{
private:
    int m_target_version = 1;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 1, "2018-01-23 10:27:01"},
        {"table_versions", 0, "2017-11-11 08:57:42"},
        {"accounts", 0, "2017-11-11 08:57:43"},
        {"characters", 2, "2018-01-23 10:16:27"},
        {"costume", 0, "2017-11-11 08:57:43"},
        {"progress", 0, "2017-11-11 08:57:43"},
        {"supergroups", 0, "2018-01-23 10:16:43"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // update database table schemas here
        // first let's add the supergroups table
        db->m_query->prepare("CREATE TABLE `supergroups` ("
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

        if(!db->m_query->exec())
            return false;

        // second: add the chardata column to the characters table
        db->m_query->prepare("ALTER TABLE 'characters' ADD 'chardata' BLOB");
        if(!db->m_query->exec())
            return false;

        // third: copy the values from columns we plan to delete
        db->m_query->prepare("SELECT * FROM 'characters'");
        if(!db->m_query->exec())
            return false;

        while(db->m_query->next())
        {
            // store in json object to save to blob
            QJsonObject char_obj;
            char_obj.insert("Level", db->m_query->value("char_level").toJsonValue());
            char_obj.insert("CombatLevel", db->m_query->value("char_level").toJsonValue()); // should have the same value as char_level
            char_obj.insert("XP", db->m_query->value("xp").toJsonValue());
            char_obj.insert("XPDebt", db->m_query->value("xpdebt").toJsonValue());
            char_obj.insert("XPPatrol", db->m_query->value("xppatrol").toJsonValue());
            char_obj.insert("Influence", db->m_query->value("inf").toJsonValue());

            QJsonObject titles_obj;
            titles_obj.insert("value0", db->m_query->value("title").toJsonValue());
            titles_obj.insert("value1", db->m_query->value("badgetitle").toJsonValue());
            titles_obj.insert("value2", db->m_query->value("specialtitle").toJsonValue());

            char_obj.insert("HasTitles", titles_obj.isEmpty());
            char_obj.insert("ThePrefix", false);
            char_obj.insert("Title", titles_obj);

            char_obj.insert("BattleCry", "");
            char_obj.insert("Description", "");
            char_obj.insert("AFK", false);
            char_obj.insert("AfkMsg", "");
            char_obj.insert("LFG", false);
            char_obj.insert("Alignment", "hero");
            char_obj.insert("LastCostumeId", db->m_query->value("last_costume_id").toJsonValue());
            char_obj.insert("Class", db->m_query->value("archetype").toJsonValue());
            char_obj.insert("Origin", db->m_query->value("origin").toJsonValue());
            char_obj.insert("MapName", db->m_query->value("current_map").toJsonValue());
            char_obj.insert("SuperGroupCostume", false);
            char_obj.insert("UsingSGCostume", false);

            db->prepareCerealObject(char_obj);
            QJsonDocument doc(char_obj);
            qCDebug(logMigration).noquote() << doc.toJson();

            // fourth: move values to new location (chardata)
            QString querytext = QString("UPDATE characters SET chardata='%1'").arg(QString(doc.toJson()));
            if(!db->m_query->exec(querytext))
                return false;
        }

        // fifth: delete columns from characters table
        // 'char_level', `archetype`, `origin`, `current_map`, `last_costume_id`,
        // `inf`, `xp`, `xpdebt`, `xppatrol`, `alignment`, `title`,
        // `badgetitle`, `specialtitle`
        QStringList cols_to_drop = {
            "char_level", "archetype", "origin", "current_map",
            "last_costume_id", "inf", "xp", "xpdebt", "xppatrol",
            "alignment", "title", "badgetitle", "specialtitle"
        };
        db->deleteColumns(QStringLiteral("characters"), cols_to_drop);

        // we're done, return true
        return true;
    }
};
