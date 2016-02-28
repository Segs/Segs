#include "SqliteDatabase.h"

#include <sqlite/sqlite3.h>
#include <ace/Log_Msg.h>
#include <ace/OS_NS_time.h>
#include <ace/OS_NS_Thread.h>
#include <map>

namespace {
struct SqliteRow : public IResultRow  {

    sqlite3_stmt *m_query;
    std::map<std::string,int> m_name_to_idx;
    // IResultRow interface
public:
    SqliteRow(sqlite3_stmt *query) {
        m_query = query;
        if(query!=NULL) {
            for (int i=0, total = sqlite3_column_count(query); i<total; ++i) {
                const char *name = sqlite3_column_name(query,i);
                // add name to the dictionary
                m_name_to_idx[name] = i;
            }
        }
    }
    int column_idx_or_error(const char *column_name) {
        std::map<std::string,int>::iterator iter = m_name_to_idx.find(column_name);
        if(iter==m_name_to_idx.end()) {
            ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: Unknown column requested: %s\n"),column_name));
            return -1;
        }
        return iter->second;
    }
    const char *getColString(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return (const char*)sqlite3_column_text(m_query,idx);
    }
    int16_t getColInt16(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return sqlite3_column_int(m_query,idx);
    }
    int32_t getColInt32(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return sqlite3_column_int(m_query,idx);
    }
    int64_t getColInt64(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return sqlite3_column_int64(m_query,idx);
    }
    int64_t getColInt64(int idx) override
    {
        if(idx<0 || idx>=sqlite3_column_count(m_query))
            return 0;
        return sqlite3_column_int64(m_query,idx);
    }
    bool getColBool(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return sqlite3_column_int(m_query,idx)!=0;
    }
    float getColFloat(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return 0;
        return sqlite3_column_double(m_query,idx);
    }
    tm getTimestamp(const char *column_name) override
    {
        struct tm ts_result;
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return ts_result;
        (void)ACE_OS::strptime((const char *)sqlite3_column_text(m_query,idx),"%Y-%m-%d %T",&ts_result);
        return ts_result;
    }
    vBinData getColBinary(const char *column_name) override
    {
        int idx = column_idx_or_error(column_name);
        if(idx==-1)
            return vBinData();
        int countbytes = sqlite3_column_bytes(m_query,idx);
        const char *bytes = (const char *)sqlite3_column_blob(m_query,idx);
        vBinData dat;
        dat.assign(bytes,bytes+countbytes);
        return dat;
    }
} ;
struct SqliteResult : public IResult {
    sqlite3_stmt *m_sqlite_query;
    sqlite3 *m_db_iface;
    bool exhausted_data = false;
    std::string m_err;
    // IResult interface
public:
    void reset() {
        if(m_sqlite_query) {
            sqlite3_reset(m_sqlite_query);
        }
        m_err.clear();
    }
    virtual ~SqliteResult() {
        reset();
    }
    const char *message() override
    {
        return m_err.c_str();
    }
    IResultRow *nextRow() override
    {
        if(!m_sqlite_query || exhausted_data)
            return NULL;
        int step_res = sqlite3_step(m_sqlite_query);
        if(step_res == SQLITE_ROW) {
            return new SqliteRow(m_sqlite_query);
        }
        if(step_res == SQLITE_DONE) {
            exhausted_data = true;
        }
        else {
            m_err = sqlite3_errmsg(m_db_iface);
        }
        return NULL;
    }
};
struct SqlitePreparedQuery : public IPreparedQuery {
    sqlite3_stmt *m_sqlite_query=nullptr;
    sqlite3 *m_db_iface=nullptr;
    bool executing;
    // IPreparedQuery interface
public:

    SqlitePreparedQuery(SqlitePreparedQuery &&from) {
        if(m_sqlite_query)
            sqlite3_finalize(m_sqlite_query);
        m_sqlite_query = nullptr;
        m_db_iface=nullptr;
        std::swap(from.m_sqlite_query,m_sqlite_query);
        std::swap(from.m_db_iface,m_db_iface);
    }
    SqlitePreparedQuery(const std::string &qr) {
        m_query_text=qr;
        executing= false;
    }
    virtual ~SqlitePreparedQuery() {
        sqlite3_finalize(m_sqlite_query);
    }
    bool execute(PreparedArgs &args, DbResults &res) override {
        if(!m_sqlite_query) {
            ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: Parameter binding - missing sqlite query\n")));
            return false;
        }
        if(executing) {
            sqlite3_reset(m_sqlite_query); // just in case - NOTE: this will reset all results that are still in use for this query
        }
        for(size_t idx=0, total = args.m_params.size(); idx<total; ++idx) {
            int bindres=0;
            if(args.m_formats[idx]==0) { // string
                bindres=sqlite3_bind_text(m_sqlite_query,idx+1,args.m_params[idx].c_str(),args.m_params[idx].size(),SQLITE_TRANSIENT);
            }
            else if(args.m_formats[idx]==1) { // integer
                if(args.m_lengths[idx]==2) { // short / int
                    bindres=sqlite3_bind_int(m_sqlite_query,idx+1,*(uint16_t*)args.m_params[idx].data());
                } else if (args.m_lengths[idx]==4) {
                    bindres=sqlite3_bind_int(m_sqlite_query,idx+1,*(uint32_t*)args.m_params[idx].data());
                } else if (args.m_lengths[idx]==8) {
                    bindres=sqlite3_bind_int64(m_sqlite_query,idx+1,*(uint64_t*)args.m_params[idx].data());
                }
                else
                    bindres=sqlite3_bind_blob(m_sqlite_query,idx+1,args.m_params[idx].data(),args.m_params[idx].size(),SQLITE_TRANSIENT);
            }
            else
                assert(!"Unknown parameter format");
            if(SQLITE_OK!=bindres) {
                ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: Parameter binding failed %s\n"),sqlite3_errmsg(m_db_iface)));
                return false;
            }
        }
        SqliteResult * sql_res = new SqliteResult;
        sql_res->m_db_iface = m_db_iface;
        sql_res->m_sqlite_query = m_sqlite_query;
        res.m_impl = sql_res;
        executing=true;
        return true;
    }
};

}
SqliteDatabase::SqliteDatabase() : m_db_iface(NULL)
{

}

SqliteDatabase::~SqliteDatabase()
{
    if(m_db_iface)
        sqlite3_close(m_db_iface);
    m_db_iface = NULL;
}

///
/// \brief SqliteDatabase::setConnectionConfiguration
/// \param host  - ignored by sqlite driver
/// \param port  - ignored by sqlite driver
/// \param db    - filename for sqlite  database
/// \param user  - ignored by sqlite driver
/// \param passw - ignored by sqlite driver
///
void SqliteDatabase::setConnectionConfiguration(const char *host, const char *port, const char *db, const char *user, const char *passw)
{
    m_filename = db;
}

bool SqliteDatabase::execQuery(const std::string &q, DbResults &res)
{
    if(res.m_impl)
        delete res.m_impl;
    assert(false);
    return false;
}

bool SqliteDatabase::execQuery(const std::string &q)
{
    assert(false);
    return false;
}

int SqliteDatabase::OpenConnection(IDataBaseCallbacks *cb)
{
    if(SQLITE_OK!=sqlite3_open(m_filename.c_str(),&m_db_iface)) {
        if(m_db_iface) {
            ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: db open failed. %s.\n"), sqlite3_errmsg(m_db_iface)));
            sqlite3_close(m_db_iface);
        }
        else {
            ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: Out of memory\n")));
        }
        exit(1); // Exit SEGS to make the user corrects the problem
    }
    ACE_DEBUG ((LM_INFO,"SQLLite connection created to %s\n",m_filename.c_str()));
    cb->on_connected(this);
    return 0;
}

int SqliteDatabase::CloseConnection()
{
    if(m_db_iface) {
        sqlite3_close(m_db_iface);
    }
    return 0;
}

int64_t SqliteDatabase::next_id(const std::string &tab_name)
{
    assert(false);
    return 0;
}

IPreparedQuery *SqliteDatabase::prepare(const std::string &query, size_t num_params)
{
    const char *unusedChars;
    std::string fixedquery=query;
    std::replace(fixedquery.begin(),fixedquery.end(),'$','?');

    SqlitePreparedQuery resp(query);
    resp.m_db_iface  = m_db_iface;
    if(SQLITE_OK!=sqlite3_prepare_v2(m_db_iface,fixedquery.c_str(),fixedquery.size(),&resp.m_sqlite_query,&unusedChars)) {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: prepare failed %s\n"),sqlite3_errmsg(m_db_iface)));
        return NULL;
    }
    return new SqlitePreparedQuery(std::move(resp));
}
IPreparedQuery *SqliteDatabase::prepareInsert(const std::string &query, size_t num_params)
{
    //
    std::string modquery = query + ";SELECT last_insert_rowid();";
    std::replace(modquery.begin(),modquery.end(),'$','?');
    const char *unusedChars;
    SqlitePreparedQuery resp(modquery);
    resp.m_db_iface  = m_db_iface;
    if(SQLITE_OK!=sqlite3_prepare_v2(m_db_iface,modquery.c_str(),modquery.size(),&resp.m_sqlite_query,&unusedChars)) {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) SQLITE_Database: prepare failed %s\n"),sqlite3_errmsg(m_db_iface)));
        return NULL;
    }
    return new SqlitePreparedQuery(std::move(resp));
}

void *SqliteDatabase::conn()
{
    assert(false);
    return 0;
}
