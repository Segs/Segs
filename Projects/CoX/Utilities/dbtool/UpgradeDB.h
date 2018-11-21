/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <vector>
#include <QtCore/QString>
#include <QtCore/QMap>

class DatabaseConfig;

// define VersionSchema QMap<table, version>
using VersionSchema = QMap<QString, int>;

struct TableVersion
{
    QString m_db_name;
    QString m_table_name;
    int     m_version;
    QString m_last_updated;
};
using TableSchema = std::vector<TableVersion>;

void doUpgrade(std::vector<DatabaseConfig> const& configs);
QStringList getTablesToUpdate(const VersionSchema &cfg, const VersionSchema &tpl);
VersionSchema getDBTableVersions(const DatabaseConfig &cfg);
