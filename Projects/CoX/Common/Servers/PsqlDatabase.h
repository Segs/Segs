/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2014 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "Database.h"
#include <libpq-fe.h>

class PSqlPreparedQuery : public IPreparedQuery
{
        PGconn *    m_conn;
        std::string m_query_name;
        size_t      m_param_count;
public:
                    PSqlPreparedQuery(Database &db) : m_conn((PGconn *)db.conn()) {}
        bool        prepare(const std::string &query,size_t num_params);
        bool        execute(PreparedArgs &args,DbResults &res);
};
class PSqlDbResults : public IResult
{
public:
        PGresult *  m_result;
        char *      m_msg;
                    PSqlDbResults();
                    ~PSqlDbResults();

        size_t      num_rows() {return PQntuples(m_result);}
        IResultRow *getRow(size_t row);
        const char *message() { return m_msg; }
        bool        isError() const {
            return (m_msg!=NULL)&&(m_msg[0]);
        }
};
class PSqlResultRow : public IResultRow {

        int         m_row;
        PGresult *  m_result;
public:
                    PSqlResultRow(PGresult *r,int row) : m_result(r), m_row(row) {}
        const char *getColString(const char *column_name);
        int16_t     getColInt16(const char *column_name);
        int32_t     getColInt32(const char *column_name);
        int64_t     getColInt64(const char *column_name);
        bool        getColBool(const char *column_name);
        float       getColFloat(const char *column_name);
        tm          getTimestamp(const char *column_name);
        vBinData    getColBinary(const char *column_name);
};
class PSqlDatabase : public Database
{
protected:
        std::string connect_string;
        PGconn *    pConnection;     // Pointer to our PostgreSQL connection structure
public:
    // Constructor/Destructor
                    PSqlDatabase();
virtual             ~PSqlDatabase();
        void        setConnectionConfiguration(const char *host,const char *port,const char *db,const char *user,const char *passw);
        bool        execQuery(const string &q,DbResults &res);
        bool        execQuery(const string &q); // for queries without results
        int         OpenConnection(IDataBaseCallbacks *c);
        int         CloseConnection(void);
        int64_t     next_id(const std::string &tab_name);
        PGconn *    get_conn() {return pConnection;}
        void *      conn() {return pConnection; }
        IPreparedQuery *prepare(const std::string &query, size_t num_params);
};
