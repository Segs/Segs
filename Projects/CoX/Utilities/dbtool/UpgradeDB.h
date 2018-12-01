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

struct TableSchema
{
    QString m_db_name;
    QString m_table_name;
    int     m_version;
    QString m_last_updated;

    inline bool operator==(const TableSchema& other) const
    {
        return  this->m_db_name == other.m_db_name &&
                this->m_table_name == other.m_table_name &&
                this->m_version == other.m_version &&
                this->m_last_updated == other.m_last_updated;
    }
};

void doUpgrade(std::vector<DatabaseConfig> const& configs);
QStringList getTablesToUpdate(const VersionSchema &cfg, const VersionSchema &tpl);
VersionSchema getDBTableVersions(const DatabaseConfig &cfg);
