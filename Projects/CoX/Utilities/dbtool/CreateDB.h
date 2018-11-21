/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "DatabaseConfig.h"
#include <vector>

class QFile;
class QSqlQuery;

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

dbToolResult createDatabases(std::vector<DatabaseConfig> const& configs);
bool deleteDb(QString const& db_file_name);
bool fileQueryDb(QFile &source_file, QSqlQuery &query);
bool dbExists(const DatabaseConfig &dbcfg);
