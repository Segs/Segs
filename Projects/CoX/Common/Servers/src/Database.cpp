/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// This module contains code to access our database where accounts and characters are stored
#include <sstream>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
// segs Includes
#include "Database.h"
#include <string.h>
// Our PostgreSQL connection information
Database::Database()
{
	pConnection=0;
};
void Database::setConnectionConfiguration(const char *host,const char *port,const char *db,const char *user,const char *passw)
{
	std::stringstream res;
	res <<"host="<<host<< " port="<<port<<" dbname="<<db<<" user="<<user<<" password="<<passw;
	connect_string=res.str();

}
int Database::OpenConnection(void) // Opens connection to the database server
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
		this->on_connected();
	}
	return 0;
}

int Database::CloseConnection(void) // Closes connection to the database server
{
	PQfinish(pConnection);    // Close our connection to the PostgreSQL db server
	ACE_DEBUG((LM_INFO,"Connection to PostgreSQL database server closed.\n"));
	pConnection=0;
	return 0;
}

bool Database::execQuery(const string &q,DbResults &res)
{
	if(res.m_result)
		PQclear(res.m_result);
	res.m_result = PQexec(pConnection,q.c_str());   // Send our query to the PostgreSQL db server to process
	if(!res.m_result)
		return false;
	res.m_msg = PQresultErrorMessage(res.m_result);

	if(res.m_msg&&res.m_msg[0])
		return false;
	return true;
}
bool Database::execQuery(const string &q)// for insert/update/delete queries, no 'real' result is returned
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

s64 Database::next_id( const std::string &tab_name )
{
    DbResults results;
    bool res=execQuery("SELECT NEXTVAL('"+tab_name+"_id_seq');",results);
    if(res==false)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database::next_id failed: %s.\n"),results.m_msg),-1);
    else if (results.num_rows()!=1)
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database::next_id returned wrong number of results.\n")),-1);
    return results.getRow(0).getColInt64("NEXTVAL");
}

Database::~Database()
{
    if(pConnection)
        CloseConnection();
}
DbResults::DbResults()
{
	m_result=0;
	m_msg = 0;
}
DbResults::~DbResults()
{
	PQclear(m_result);
}
DbResultRow DbResults::getRow(size_t row)
{
	if(row>=num_rows())
		return DbResultRow(); // uninitialized row is returned here
	return DbResultRow(m_result,row);
}



const char *DbResultRow::getColString(const char *column_name)
{
	if(!m_result)
		return NULL;
	int field_no = PQfnumber(m_result,column_name);
	if(field_no<0)
		return NULL;
	return PQgetvalue(m_result,m_row,field_no);
}
s16 DbResultRow::getColInt16(const char *column_name)
{
	return (s16)getColInt32(column_name);
}
bool DbResultRow::getColBool(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),false);
	}
	return toupper(res[0])=='T';
}
s32 DbResultRow::getColInt32(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);
	}
	s32 result;
	sscanf(res,ACE_INT32_FORMAT_SPECIFIER,&result);
	return result;
}
s64 DbResultRow::getColInt64(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),-1);
	}
	s64 result;
	sscanf(res,ACE_INT64_FORMAT_SPECIFIER,&result);
	return result;
}
float DbResultRow::getColFloat(const char *column_name)
{
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0); //
	}

	return (float)atof(res);
}
size_t DbResultRow::getColIntArray(const char *column_name,u32 *arr,size_t arr_size)
{
	size_t idx=0;
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
	}
	while(res && idx<arr_size)
	{
		res++;
		arr[idx++]=strtol(res,0,0);
		res=strchr(res,',');
	}
	return idx;
}
size_t DbResultRow::getColFloatArray(const char *column_name,float *arr,size_t arr_size)
{
	size_t idx=0;
	const char *res = getColString(column_name);
	if(!res)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name),0);
	}
	while(res && idx<arr_size)
	{
		res++;
		arr[idx++]=(float)atof(res);
		res=strchr(res,',');
	}
	return idx;
}

struct tm DbResultRow::getTimestamp( const char *column_name )
{
    struct tm ts_result={};
    const char *res = getColString(column_name);
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("(%P|%t) Database: unknown column:%s.\n"), column_name));
        return ts_result;
    }
    (void)ACE_OS::strptime(res,"%Y-%m-%d %T",&ts_result);
    return ts_result;
}

bool PreparedQuery::prepare(const std::string &query,size_t num_params)
{
    char prep_name[32];
    m_param_count = num_params;
    ACE_OS::unique_name(this,prep_name,32);
    m_query_name=std::string("segs_q")+prep_name;
    PGresult * q_res = PQprepare(m_conn,m_query_name.c_str(),query.c_str(),m_param_count,0);
    char *err = PQresultErrorMessage(q_res);
    bool res = !(err&&err[0]); // return false if err is set
    PQclear(q_res);
    return res;
}

bool PreparedQuery::execute(PreparedArgs &args,DbResults &res )
{
    ACE_ASSERT(args.m_params.size()==m_param_count);
    std::vector<const char *> values;
    for(size_t idx=0; idx<args.m_params.size(); ++idx)
        values.push_back(args.m_params[idx].c_str());
    res.m_result=PQexecPrepared(m_conn,m_query_name.c_str(),m_param_count,&values[0],&args.m_lengths[0],&args.m_formats[0],1);
    if(!res.m_result)
        return false;
    res.m_msg = PQresultErrorMessage(res.m_result);

    if(res.m_msg&&res.m_msg[0])
        return false;
    return true;
}
void PreparedArgs::set_param( size_t idx,const u8 *bytes,size_t len,bool binary )
{
    m_params.push_back(std::string((char *)bytes,len));
    m_lengths.push_back(len);
    m_formats.push_back(binary ? 1 : 0);
}

void PreparedArgs::set_param( size_t idx,const std::string &str )
{
    m_params.push_back(str);
    m_lengths.push_back(str.size());
    m_formats.push_back(0);
}

void PreparedArgs::set_param( size_t idx,u16 v )
{
    u16 rv = ACE_HTONS(v);
    m_params.push_back(std::string((char *)&rv,2));
    m_lengths.push_back(2);
    m_formats.push_back(1);
}
void PreparedArgs::set_param( size_t idx,u32 v )
{
    u32 rv = ACE_HTONL(v);
    m_params.push_back(std::string((char *)&rv,4));
    m_lengths.push_back(4);
    m_formats.push_back(1);
}
