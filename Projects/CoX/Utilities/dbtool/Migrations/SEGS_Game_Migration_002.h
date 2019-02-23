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
            QJsonObject charObject = db->m_query->value(4).toJsonObject();
            charObject.insert("LastOnline", "");

            // fourth: move values to new location (chardata and entdata)
            QJsonDocument chardoc(charObject);
            qDebug().noquote() << chardoc.toJson();

            QJsonObject entObject;
            entObject.insert("OriginIdx", 1); // TODO: how to actually determine this?
            entObject.insert("ClassIdx", 1); // TODO: how to actually determine this?
            entObject.insert("Type", 2); // Player
            entObject.insert("Idx", 0);
            entObject.insert("dbID", 0);

            entObject = db->prepareBlob(entObject); // required by cereal
            QJsonDocument entdoc(entObject);
            qDebug().noquote() << entdoc.toJson(); // print output for debug

            QString querytext = QString("UPDATE characters SET chardata='%1', entitydata='%1'")
                    .arg(QString(chardoc.toJson()))
                    .arg(QString(entdoc.toJson()));
            if(!db->m_query->exec(querytext))
                return false;
        }
        // TODO: copy data to entitydata
        /*
         *  chardata created with serialized blob
         *  no version in this serialized blob
            archive(cereal::make_nvp("Level",cd.m_level));
            archive(cereal::make_nvp("CombatLevel",cd.m_combat_level));
            archive(cereal::make_nvp("XP",cd.m_experience_points));
            archive(cereal::make_nvp("Debt",cd.m_experience_debt));
            archive(cereal::make_nvp("PatrolXP",cd.m_experience_patrol));
            archive(cereal::make_nvp("Influence",cd.m_influence));
            archive(cereal::make_nvp("HasTitles",cd.m_has_titles));
            archive(cereal::make_nvp("ThePrefix",cd.m_has_the_prefix));
            auto *titles = &cd.m_titles;
            for(int i=0; i<3; ++i) {
                sprintf(buf,"Title-%d",i);
                archive(cereal::make_nvp(buf,titles[i]));
            }
            archive(cereal::make_nvp("BattleCry",cd.m_battle_cry));
            archive(cereal::make_nvp("Description",cd.m_character_description));
            archive(cereal::make_nvp("AFK",cd.m_afk));
            archive(cereal::make_nvp("AfkMsg",cd.m_afk_msg));
            archive(cereal::make_nvp("LFG",cd.m_lfg));
            archive(cereal::make_nvp("SuperGroupID",cd.m_supergroup_id));
            archive(cereal::make_nvp("Alignment",cd.m_alignment));
            archive(cereal::make_nvp("LastCostumeID",cd.m_last_costume_id));
            archive(cereal::make_nvp("Class",cd.m_class_name));
            archive(cereal::make_nvp("Origin",cd.m_origin_name));
            archive(cereal::make_nvp("MapName",cd.m_mapName));
            archive(cereal::make_nvp("AccountID",cd.m_account_id));
            archive(cereal::make_nvp("dbID",cd.m_db_id));
            archive(cereal::make_nvp("PowerLevel",cd.m_power_level));

         *  entitydata created with serialized blob
         *  no version in this serialized blob
            archive(cereal::make_nvp("OriginIdx",ed.m_origin_idx));
            archive(cereal::make_nvp("ClassIdx",ed.m_class_idx));
            archive(cereal::make_nvp("Type",ed.m_type));
            archive(cereal::make_nvp("Idx",ed.m_idx));
            archive(cereal::make_nvp("dbID",ed.m_db_id));
        */

        // we're done, return true
        return true;
    }
};
