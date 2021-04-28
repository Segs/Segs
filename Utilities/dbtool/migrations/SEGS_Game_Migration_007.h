/*
 * SEGS - Super Entity Game Server - dbtool Migration
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBMigrationStep.h"

class SEGS_Game_Migration_007 : public DBMigrationStep
{
private:
    int m_target_version = 7;
    QString m_name = SEGS_GAME_DB_NAME;
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", 7, "2018-05-03 17:52:33"},
        {"accounts", 1, "2017-05-03 12:56:03"},
        {"characters", 8, "2018-05-04 14:58:27"},
        {"supergroups", 1, "2018-05-03 12:56:43"},
    };

public:
    int getTargetVersion() const override { return m_target_version; }
    QString getName() const override { return m_name; }
    std::vector<TableSchema> getTableVersions() const override { return m_table_schemas; }

    // execute the migration
    bool execute(DBConnection *db) override
    {
        // first: select the data from characters table
        QString select_qry = "SELECT * FROM 'characters'";
        if(!db->runQuery(select_qry))
            return false;

        // second column copy data over to character data blob
        while(db->m_query->next())
        {
            QJsonObject char_obj = db->loadBlob("chardata");
            char_obj["cereal_class_version"] = 8; // set to 8

            QJsonObject cur_attribs;
            for(int i = 0; i < 24; ++i)
            {
                // fill 24 Damage Types
                QString keyname = QString("DamageType%1").arg(i, 2, 10, QChar('0'));
                cur_attribs.insert(keyname, 0);
            }

            cur_attribs.insert("HitPoints", db->m_query->value("hitpoints").toJsonValue());
            cur_attribs.insert("Endurance", db->m_query->value("endurance").toJsonValue());
            cur_attribs.insert("ToHit", 0);

            for(int i = 0; i < 24; ++i)
            {
                // fill 24 Defense Types
                QString keyname = QString("DefenseType00%1").arg(i, 2, 10, QChar('0'));
                cur_attribs.insert(keyname, 0);
            }

            cur_attribs.insert("Defense", 0);
            cur_attribs.insert("Evade", 0);
            cur_attribs.insert("SpeedRunning", 0);
            cur_attribs.insert("SpeedFlying", 0);
            cur_attribs.insert("SpeedSwimming", 0);
            cur_attribs.insert("SpeedJumping", 0);
            cur_attribs.insert("JumpHeight", 0);
            cur_attribs.insert("MovementControl", 0);
            cur_attribs.insert("MovementFriction", 0);
            cur_attribs.insert("Stealth", 0);
            cur_attribs.insert("StealthRadius", 0);
            cur_attribs.insert("PerceptionRadius", 0);
            cur_attribs.insert("Regeneration", 0);
            cur_attribs.insert("Recovery", 0);
            cur_attribs.insert("ThreatLevel", 0);
            cur_attribs.insert("Taunt", 0);
            cur_attribs.insert("Confused", 0);
            cur_attribs.insert("Afraid", 0);
            cur_attribs.insert("Held", 0);
            cur_attribs.insert("Immobilized", 0);
            cur_attribs.insert("Stunned", 0);
            cur_attribs.insert("Sleep", 0);
            cur_attribs.insert("Fly", 0);
            cur_attribs.insert("Jumppack", 0);
            cur_attribs.insert("Teleport", 0);
            cur_attribs.insert("Untouchable", 0);
            cur_attribs.insert("Intangible", 0);
            cur_attribs.insert("OnlyAffectsSelf", 0);
            cur_attribs.insert("Knockup", 0);
            cur_attribs.insert("Knockback", 0);
            cur_attribs.insert("Repel", 0);
            cur_attribs.insert("Accuracy", 0);
            cur_attribs.insert("Radius", 0);
            cur_attribs.insert("Arc", 0);
            cur_attribs.insert("Range", 0);
            cur_attribs.insert("TimeToActivate", 0);
            cur_attribs.insert("RechargeTime", 0);
            cur_attribs.insert("InterruptTime", 0);
            cur_attribs.insert("EnduranceDiscount", 0);

            char_obj.insert("CurrentAttribs", cur_attribs); // cereal objects are wrapped in key 'value0'

            QString chardoc = db->saveBlob(char_obj);
            qCDebug(logMigration).noquote() << chardoc; // print output for debug

            QString querytext = QString("UPDATE characters SET chardata='%1'")
                    .arg(chardoc);
            if(!db->runQuery(querytext))
                return false;
        }

        // the rest of this update just simplified our table foreign keys
        QStringList queries = {
            "ALTER TABLE characters DROP FOREIGN KEY account_id",        // drop key because we're going to change it
            "ALTER TABLE characters ADD FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE",
        };

        if(!db->runQueries(queries))
            return false;

        // delete columns from tables
        db->deleteColumns(QStringLiteral("supergroups"), QStringList("supergroup_id"));
        db->deleteColumns(QStringLiteral("accounts"), QStringList("account_id"));
        db->deleteColumns(QStringLiteral("characters"), QStringList({ "char_level", "archetype" }));

        // we're done, return true
        return true;
    }
};
