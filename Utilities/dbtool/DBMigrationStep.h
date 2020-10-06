/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "DBConnection.h"
#include "Components/Logging.h"

#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

constexpr const QLatin1String SEGS_DB_NAME("segs",4);
constexpr const QLatin1String SEGS_GAME_DB_NAME("segs_game",9);

class DBMigrationStep
{
private:
    int m_target_version = 0;
    QString m_name;
    // TODO: should table_schemas go away? We can programatically update
    // the version numbers, so we're simply being overly cautious with
    // version control. I'm undecided.
    std::vector<TableSchema> m_table_schemas = {
        {"db_version", m_target_version, "2018-02-15 10:23:43"}
    };

public:
    DBMigrationStep();

    virtual int getTargetVersion() const { return m_target_version; }
    virtual QString getName() const { return m_name; }
    virtual std::vector<TableSchema> getTableVersions() const { return m_table_schemas; }

    virtual bool canRun(DBConnection *db, int cur_version);
    virtual bool execute(DBConnection * /*db*/) { return false; }
    virtual bool cleanup(DBConnection *db);
};
