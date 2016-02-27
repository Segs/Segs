/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Database.h"


class SqliteDatabase : public Database
{
public:
    SqliteDatabase();

    // Database interface
public:
    void setConnectionConfiguration(const char *host, const char *port, const char *db, const char *user, const char *passw) override;
    bool execQuery(const std::string &q, DbResults &res) override;
    bool execQuery(const std::string &q) override;
    int OpenConnection(IDataBaseCallbacks *) override;
    int CloseConnection() override;
    int64_t next_id(const std::string &tab_name) override;
    IPreparedQuery *prepare(const std::string &query, size_t num_params) override;
    void *conn() override;
};
