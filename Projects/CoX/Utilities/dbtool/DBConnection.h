/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "DatabaseConfig.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <vector>
#include <memory>

class QFile;
class DatabaseConfig;
struct UpgradeHook;

// TODO: This should go away and merge with DBConnection or DatabaseConfig
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

// Return value enumeration
enum class dbToolResult : int
{
    SUCCESS             = 0,
    SETTINGS_MISSING,
    DBFOLDER_MISSING,
    NOT_FORCED,
    NOT_ENOUGH_PARAMS,
    SQLITE_DB_MISSING,
    DB_RM_FAILED,
    DB_CONN_FAILED,
    QUERY_FAILED,
    QUERY_PREP_FAILED,
    USERNAME_TAKEN
};

class DBConnection
{
public:
    DatabaseConfig m_config;
    QSqlDatabase m_db;
    std::unique_ptr<QSqlQuery> m_query;

    DBConnection(const DatabaseConfig &cfg);
    ~DBConnection();

    void open();
    void close();
    bool isConnected();
    QString getName() { return m_config.m_name; } // "segs" or "segs_game"

    // DBConnection_AddUser.cpp
    dbToolResult addAccount(const QString &username, const QString &password, uint16_t access_level);

    // DBConnection_Create.cpp
    dbToolResult createDB();
    bool deleteDB();
    bool checkTableVersions(const std::vector<TableSchema> &table_schemas);
    bool updateTableVersions(std::vector<TableSchema> &table_schemas);
    bool runQueryFromFile(QFile &source_file);

    // DBConnection_Upgrade.cpp
    void runUpgrades();
    TableSchema getDBVersion();
    bool checkVersionAndUpgrade(const TableSchema &cur_version);
    bool runUpgradeHooks(const std::vector<UpgradeHook> &hooks_to_run);
};
