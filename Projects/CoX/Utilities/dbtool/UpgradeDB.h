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

struct TableSchema
{
    QString m_db_name;
    int     m_version;

    inline bool operator==(const TableSchema& other) const
    {
        return  this->m_db_name == other.m_db_name &&
                this->m_version == other.m_version;
    }
};
using DBSchemas = std::vector<TableSchema>;

void runUpgrades(std::vector<DatabaseConfig> const& configs);
TableSchema getDBVersion(const DatabaseConfig &cfg);
bool checkVersionAndUpgrade(const DatabaseConfig &cfg, const TableSchema &cur_version);
bool runQuery(const DatabaseConfig &cfg, const QString query_text);
