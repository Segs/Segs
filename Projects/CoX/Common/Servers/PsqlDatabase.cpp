#include "PsqlDatabase.h"

#include <sstream>
#include <cassert>
#include <string.h>
#ifdef LINUX // forgive me brothers for i have sinned
#include <inttypes.h>
#else
#define SCNd32 "d"
#define SCNd64 "I64d"
#endif
#include <ace/Log_Msg.h>
#include <ace/OS_NS_time.h>
#include <ace/OS_NS_Thread.h>
#include <libpq-fe.h>

// Our PostgreSQL connection information
PSqlDatabase::PSqlDatabase()
{
    pConnection=0;
}
void PSqlDatabase::setConnectionConfiguration(const char *host,const char *port,const char *db,const char *user,const char *passw)
{
    std::stringstream res;
    res <<"host="<<host<< " port="<<port<<" dbname="<<db<<" user="<<user<<" password="<<passw;
    connect_string=res.str();

}
int PSqlDatabase::OpenConnection(IDataBaseCallbacks *c) // Opens connection to the database server
{
    pConnection = PQconnectdb(connect_string.c_str());     // Connect with our string defined above

    if (PQstatus(pConnection) != CONNECTION_OK)   // If the connection did not go well, let us know
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: connection to %s failed. %s.\n"), PQhost(pConnection), PQerrorMessage(pConnection)));
        exit(1); // Exit SEGS to make the user corrects the problem
    }
    else
    {
        ACE_DEBUG ((LM_INFO,"Connected to %s database server version %i.\n",PQdb(pConnection), PQserverVersion(pConnection)));
        c->on_connected(this);
    }
    return 0;
}

int PSqlDatabase::CloseConnection(void) // Closes connection to the database server
{
    PQfinish(pConnection);    // Close our connection to the PostgreSQL db server
    ACE_DEBUG((LM_INFO,"Connection to PostgreSQL database server closed.\n"));
    pConnection=0;
    return 0;
}

bool PSqlDatabase::execQuery(const std::string &q,DbResults &res)
{
    if(!res.m_impl)
        res.m_impl = new PSqlDbResults;
    PSqlDbResults *p_res = (PSqlDbResults *)res.m_impl;
    if(p_res->m_result)
        PQclear(p_res->m_result);
    p_res->m_result = PQexec(pConnection,q.c_str());   // Send our query to the PostgreSQL db server to process
    if(!p_res->m_result)
        return false;
    p_res->m_msg = PQresultErrorMessage(p_res->m_result);
    bool q_error = p_res->isError();
    return q_error ? false : true;
}
bool PSqlDatabase::execQuery(const std::string &q)// for insert/update/delete queries, no 'real' result is returned
{
    PGresult *m_result = PQexec(pConnection,q.c_str());   // Send our query to the PostgreSQL db server to process
    if(!m_result)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: query %s failed. ERR_FATAL.\n"), q.c_str()),false);
    char *err = PQresultErrorMessage(m_result);
    if(err&&err[0])
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: query %s failed. \n %s \n"), q.c_str(),err));
        PQclear(m_result);
        return false;
    }
    PQclear(m_result);
    return true;
}

int64_t PSqlDatabase::next_id( const std::string &tab_name )
{
    DbResults results;
    bool res=execQuery("SELECT NEXTVAL('"+tab_name+"_id_seq');",results);
    if(res==false)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database::next_id failed: %s.\n"),results.message()),-1);
    else {
        DbResultRow r = results.nextRow();
        if(r.valid()) {
            return r.getColInt64("NEXTVAL");
        }
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database::next_id returned wrong number of results.\n")),-1);
    }
    return -1;
}

PSqlDatabase::~PSqlDatabase()
{
    if(pConnection)
        CloseConnection();
}

IPreparedQuery *PSqlDatabase::prepareInsert(const std::string &query,size_t num_params)
{
    std::string modquery = query + " returning id";

    PSqlPreparedQuery *res = new PSqlPreparedQuery(*this);
    if(res->prepare(modquery,num_params))
        return res;
    delete res;
    return NULL;
}

IPreparedQuery *PSqlDatabase::prepare(const std::string &query,size_t num_params)
{
    PSqlPreparedQuery *res = new PSqlPreparedQuery(*this);
    if(res->prepare(query,num_params))
        return res;
    delete res;
    return NULL;
}

bool PSqlPreparedQuery::prepare(const std::string &query,size_t num_params)
{
    char prep_name[32];
    m_query_text = query;
    m_param_count = num_params;
    ACE_OS::unique_name(this,prep_name,32);
    m_query_name=std::string("segs_q")+prep_name;
    PGresult * q_res = PQprepare(m_conn,m_query_name.c_str(),query.c_str(),m_param_count,0);
    char *err = PQresultErrorMessage(q_res);
    bool res = !(err&&err[0]); // return false if err is set
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: PQprepare(): %s.\n"), err));
    }
    PQclear(q_res);
    return res;
}

bool PSqlPreparedQuery::execute(PreparedArgs &args,DbResults &res )
{
    assert(args.m_params.size()==m_param_count);
    std::vector<const char *> values;
    for(size_t idx=0; idx<args.m_params.size(); ++idx)
        values.push_back(args.m_params[idx].c_str());
    if(!res.m_impl)
        res.m_impl = new PSqlDbResults;
    PSqlDbResults *p_res = (PSqlDbResults *)res.m_impl;

    p_res->m_result=PQexecPrepared(m_conn,m_query_name.c_str(),m_param_count,&values[0],&args.m_lengths[0],&args.m_formats[0],1);
    if(!p_res->m_result)
        return false;
    p_res->m_msg = PQresultErrorMessage(p_res->m_result);
    bool q_error = p_res->isError();
    return q_error ? false : true;
}

int64_t PSqlPreparedQuery::executeInsert(PreparedArgs &args, DbResults &res)
{
    if(!execute(args,res))
        return -1;
    return res.nextRow().getColInt64(0);
}



const char *PSqlResultRow::getColString(const char *column_name)
{
    if(!m_result)
        return NULL;
    int field_no = PQfnumber(m_result,column_name);
    if(field_no<0)
        return NULL;
    return PQgetvalue(m_result,m_row,field_no);
}

int16_t PSqlResultRow::getColInt16(const char *column_name)
{
    return (int16_t)getColInt32(column_name);
}

int32_t PSqlResultRow::getColInt32(const char *column_name)
{
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);
    }
    int32_t result;
    sscanf(res, "%" SCNd32,&result);
    return result;
}


int64_t PSqlResultRow::getColInt64(const char *column_name)
{
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);
    }
    int64_t result;
    sscanf(res,"%" SCNd64,&result);
    return result;
}
int64_t PSqlResultRow::getColInt64(int colidx)
{
    const char *res = PQgetvalue(m_result,m_row,colidx);
    if(!res)
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column idx:%d.\n"), colidx),-1);
    }
    int64_t result;
    sscanf(res,"%" SCNd64,&result);
    return result;
}
IResultRow::vBinData PSqlResultRow::getColBinary(const char *column_name)
{
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name));
        return std::vector<uint8_t>();
    }
    size_t unescaped_len;
    uint8_t *binary_password = PQunescapeBytea((uint8_t *)res,&unescaped_len);
    std::vector<uint8_t> bin((uint8_t *)binary_password,(uint8_t *)binary_password+unescaped_len);
    PQfreemem(binary_password);
    return bin;
}
bool PSqlResultRow::getColBool(const char *column_name)
{
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),false);
    }
    return toupper(res[0])=='T';
}

float PSqlResultRow::getColFloat(const char *column_name)
{
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0); //
    }

    return (float)atof(res);
}

QDateTime PSqlResultRow::getTimestamp(const char *column_name)
{
    QDateTime ts_result;
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name));
        return ts_result;
    }
    ts_result = QDateTime::fromString(res,"yyyy-M-d h:m:s");
    return ts_result;
}

//size_t DbResultRow::getColIntArray(const char *column_name,uint32_t *arr,size_t arr_size)
//{
//    size_t idx=0;
//    const char *res = getColString(column_name);
//    if(!res)
//    {
//        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
//    }
//    while(res && idx<arr_size)
//    {
//        res++;
//        arr[idx++]=strtol(res,0,0);
//        res=strchr(res,',');
//    }
//    return idx;
//}
//size_t DbResultRow::getColFloatArray(const char *column_name,float *arr,size_t arr_size)
//{
//    size_t idx=0;
//    const char *res = getColString(column_name);
//    if(!res)
//    {
//        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
//    }
//    while(res && idx<arr_size)
//    {
//        res++;
//        arr[idx++]=(float)atof(res);
//        res=strchr(res,',');
//    }
//    return idx;
//}



PSqlDbResults::PSqlDbResults()
{
    m_current_row = 0;
    m_result=NULL;
    m_msg = NULL;

}

PSqlDbResults::~PSqlDbResults()
{
    PQclear(m_result);
}

IResultRow *PSqlDbResults::nextRow()
{
    if(m_current_row>=PQntuples(m_result))
        return NULL;
    return new PSqlResultRow(m_result,m_current_row++);
}
