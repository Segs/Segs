/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "UpgradeHooks.h"
#include "DatabaseConfig.h"

#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/*
 *
 *
 * THIS FILE WILL BE DELETED ONCE DBMigrations are auto imported
 *
 *
 */

// handlers for segs db upgrades
// please add new handlers in numerical order
bool upgradeHandler_accounts_001(DBConnection *db, std::unique_ptr<QSqlQuery> &query);

// handlers for segs_game db upgrades
// please add new handlers in numerical order
bool upgradeHandler_game_001(DBConnection *db, std::unique_ptr<QSqlQuery> &query);
bool upgradeHandler_game_002(DBConnection *db, std::unique_ptr<QSqlQuery> &query);

// upgrade hook vector
// please add new handlers in numberical order
const std::vector<UpgradeHook> g_segs_upgrade_hooks = {
    /* segs database changes */
    {{"segs",   1},         upgradeHandler_accounts_001},
    /* segs_game database Changes */
    {{"segs_game",  10},    upgradeHandler_game_001},
    {{"segs_game",  11},    upgradeHandler_game_002},
};

/*
 * segs db upgradeHandlers
 * please add handlers in numerical order
 */
bool upgradeHandler_accounts_001(DBConnection *db, std::unique_ptr<QSqlQuery> &query)
{
    qInfo() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

    // update database table schemas here
    QStringList queries = {
        "ALTER TABLE 'accounts' ADD 'salt' BLOB",
        // NOTE: there's a typo in db_version version number, which was set to 0 in the update
        "UPDATE 'table_versions' SET version='0', last_update='2018-01-06 16:27:58' WHERE table_name='db_version'",
        "UPDATE 'table_versions' SET version='1', last_update='2018-01-06 11:18:01' WHERE table_name='accounts'",
    };

    for(QString &q : queries)
    {
        if(!query->exec(q))
            return false;
    }

    return true; // if successful
}

/*
 * segs_game db upgradeHandlers
 * please add handlers in numerical order
 */
bool upgradeHandler_game_001(DBConnection *db, std::unique_ptr<QSqlQuery> &query)
{
    // TEST migration from segs_game v0 to v1
    qInfo() << "PERFORMING UPGRADE 001 on" << db->m_config.m_db_path;

    query->prepare("SELECT * FROM 'characters'");
    if(!query->exec())
        return false;

    while(query->next())
    {
        QVariantMap work_area;
        work_area["char_level"] = query->value(4);
        work_area["archetype"] = query->value(5);
        work_area["origin"] = query->value(6);
        work_area["current_map"] = query->value(8);
        work_area["last_costume_id"] = query->value(9);
        work_area["inf"] = query->value(13);
        work_area["xp"] = query->value(14);
        work_area["xpdebt"] = query->value(15);
        work_area["xppatrol"] = query->value(16);
        work_area["alignment"] = query->value(17);
        work_area["title"] = query->value(24);
        work_area["badgetitle"] = query->value(25);
        work_area["specialtitle"] = query->value(26);

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
        QString querytext = QString("UPDATE characters SET chardata='%1'").arg(QString(doc.toJson()));
        if(!query->exec(querytext))
            return false;
    }

    /*
    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!query->exec(querytext))
        return false;

    while(query->next())
    {
        int cur_version = query->value(0).toInt();

        querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query->exec(querytext))
            return false;
    }

    */
    return true; // if successful
}

bool upgradeHandler_game_002(DBConnection *db, std::unique_ptr<QSqlQuery> &query)
{
    /*
    qInfo() << "PERFORMING UPGRADE 002 on" << db->m_config.m_db_path;

    QString querytext = "SELECT version FROM table_versions WHERE table_name='db_version'";
    if(!query->exec(querytext))
        return false;

    while(query->next())
    {
        int cur_version = query->value(0).toInt();

        querytext = QString("UPDATE table_versions SET version='%1' WHERE table_name='db_version'").arg(cur_version + 1);
        if(!query->exec(querytext))
            return false;
    }

    */
    return true; // if successful
}
