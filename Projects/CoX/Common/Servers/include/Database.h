/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Database.h 301 2006-12-26 15:50:44Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef DATABASE_H
#define DATABASE_H

// ACE Logging
#include <string>
#include "types.h"
#include <libpq-fe.h>
#include <vector>
#include <string>
using std::vector;
using std::string;
class DbResultRow
{
protected:
	PGresult *m_result;
	size_t m_row;
public:
	DbResultRow(PGresult *a,size_t r) : m_result(a),m_row(r){}
	DbResultRow() : m_result(0),m_row(0){}

	const char *		getColString(const char *column_name);
			s16			getColInt16(const char *column_name);
			s32			getColInt32(const char *column_name);
			s64			getColInt64(const char *column_name);
			bool		getColBool(const char *column_name);
			float		getColFloat(const char *column_name);
			size_t		getColIntArray(const char *column_name,u32 *arr,size_t arr_size); // returns the actual number of values
			size_t		getColFloatArray(const char *column_name,float *arr,size_t arr_size);
            struct tm   getTimestamp(const char *column_name);
};
class DbResults
{
public:
	PGresult *m_result;
	char *m_msg;
	DbResults();
	~DbResults();

	size_t num_rows() {return PQntuples(m_result);};
	DbResultRow getRow(size_t row);
};
class Database
{
protected:
	    std::string connect_string;
	    PGconn *    pConnection;     // Pointer to our PostgreSQL connection structure
public:
	// Constructor/Destructor
    	                Database();
                        virtual ~Database();
	    void            setConnectionConfiguration(const char *host,const char *db,const char *user,const char *passw);
	    bool            execQuery(const string &q,DbResults &res);
	    bool            execQuery(const string &q); // for queries without results
	    int             OpenConnection(void);
	    int             CloseConnection(void);
        s64             next_id(const std::string &tab_name);
        PGconn *        get_conn() {return pConnection;}
virtual void            on_connected() = 0;
};
struct PreparedArgs
{
    std::vector<std::string> m_params;
    std::vector<int>        m_lengths;
    std::vector<int>        m_formats;
    void                    set_param(size_t idx,const u8 *bytes,size_t len,bool binary);
    void                    set_param(size_t idx,const std::string &str);
    void                    set_param(size_t idx,u16);
    void                    set_param(size_t idx,u32);
};
class PreparedQuery
{
    PGconn *                m_conn;
    std::string             m_query_name;
    size_t                  m_param_count;
public:
                            PreparedQuery(Database &db) : m_conn(db.get_conn()) {};
    bool                    prepare(const std::string &query,size_t num_params);
    bool                    execute(PreparedArgs &args,DbResults &res);
};
class DbTransactionGuard
{
    Database &m_db;
    bool proper_commit;
public:
    DbTransactionGuard(Database &db) : m_db(db),proper_commit(false)
    {
        m_db.execQuery("BEGIN;");
    }
    void commit() {proper_commit=true;}
    ~DbTransactionGuard()
    {
        if(proper_commit)
            m_db.execQuery("COMMIT;");
        else
            m_db.execQuery("ROLLBACK;");
    }

};
#endif // DATABASE_H
