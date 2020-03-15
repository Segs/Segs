/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.dev/
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
class QJsonObject;
class DatabaseConfig;
class DBMigrationStep;

struct ColumnSchema
{
    QString m_name;
    QString m_data_type;
};

struct TableSchema
{
    QString m_table_name;
    int     m_version;
    QString m_last_updated;

    inline bool operator==(const TableSchema& other) const
    {
        return  this->m_table_name == other.m_table_name &&
                this->m_version == other.m_version;
    }
};
using DBSchemas = std::vector<TableSchema>;

// Return value enumeration
enum class DBToolResult : int
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
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_query;

    DBConnection(const DatabaseConfig &cfg);
    ~DBConnection();

    void open();
    void close();
    QString getName() const { return m_config.m_short_name; } // "segs.db" or "segs_game.db"

    // DBConnection_AddUser.cpp
    DBToolResult    addAccount(const QString &username, const QString &password, uint16_t access_level);

    // DBConnection_Create.cpp
    DBToolResult    createDB();
    bool            deleteDB();
    bool            runQueryFromFile(QFile &source_file);

    // DBConnection_Helpers.cpp
    bool            isConnected();
    bool            runQuery(const QString &q);
    bool            runQueries(const QStringList &qlist);
    bool            deleteColumnsCommon(const QString &tablename, const QStringList &cols_to_remove);
    bool            deleteColumnsSqlite(const QString &tablename, const QStringList &cols_to_remove);
    bool            deleteColumns(const QString &tablename, const QStringList &cols_to_remove);
    bool            deleteColumn(const QString &tablename, const QString &col_to_remove);
    bool            getColumnsFromTable(const QString &tablename, std::vector<ColumnSchema> &old_cols);
    void            prepareCerealArray(QJsonObject &obj);
    void            prepareCerealObject(QJsonObject &obj);
    QJsonObject     loadBlob(const QString &column_name);
    QString         saveBlob(QJsonObject &obj);

    // DBConnection_Upgrade.cpp
    void            runUpgrades();
    int             getDBVersion();
    int             getFinalMigrationVersion(std::vector<DBMigrationStep *> &migrations, const QString &db_name);
    bool            updateTableVersions(const DBSchemas &table_schemas);
};

