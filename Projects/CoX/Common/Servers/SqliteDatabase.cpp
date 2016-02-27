#include "SqliteDatabase.h"

SqliteDatabase::SqliteDatabase()
{

}


void SqliteDatabase::setConnectionConfiguration(const char *host, const char *port, const char *db, const char *user, const char *passw)
{
    assert(false);
}

bool SqliteDatabase::execQuery(const std::string &q, DbResults &res)
{
    assert(false);
    return false;
}

bool SqliteDatabase::execQuery(const std::string &q)
{
    assert(false);
    return false;
}

int SqliteDatabase::OpenConnection(IDataBaseCallbacks *)
{
    assert(false);
    return 0;
}

int SqliteDatabase::CloseConnection()
{
    assert(false);
    return 0;
}

int64_t SqliteDatabase::next_id(const std::string &tab_name)
{
    assert(false);
    return 0;
}

IPreparedQuery *SqliteDatabase::prepare(const std::string &query, size_t num_params)
{
    assert(false);
    return 0;
}

void *SqliteDatabase::conn()
{
    assert(false);
    return 0;
}
